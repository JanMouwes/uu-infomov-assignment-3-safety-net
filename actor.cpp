#include "precomp.h"
#include "game.h"

// Tank constructor
Tank::Tank(Sprite* s, int2 p, int2 t, uint f, int a)
{
	// create sprite instance based on existing sprite
	// sprite = SpriteInstance(s);
	// set intial orientation / sprite visual.frame; 0: north; 64: east; 128: south; 192: east
	// visual.frame = f;
	visual = VisualComponent {SpriteInstance(s), f};

	// create the static array of directions if it doesn't exist yet
	if (directions == nullptr)
	{
		directions = new float2[256];
		for (int i = 0; i < 256; i++) directions[i] = make_float2(sinf(i * PI / 128), -cosf(i * PI / 128));
	}

	// physical.pos = make_float(p);
	// set direction based on specified orientation
	// dir = directions[frame];
	spatial = {make_float2(p), directions[visual.frame]};
	// target = make_float2(t);
	target =  {make_float2(t)};
	// assign tank to the specified army
	// army = a;
	attack = AttackComponent { a, 0 };
	collision = CollisionComponent { false };
}

// Tank::Tick : tank behaviour
bool Tank::Tick()
{
	bool isAlive = TickCollision();
	if (!isAlive) return isAlive;

	TickAttack();

	TickPhysics();
	
	// tanks never die
	return true;
}

bool Tank::TickCollision()
{
	return true;
}

void Tank::TickAttack()
{
}

void Tank::TickPhysics()
{
	// accumulate forces for steering left or right
	// 1. target attracts
	float2 toTarget = normalize( target.target - spatial.pos );
	float2 toRight = make_float2( -spatial.dir.y, spatial.dir.x );
	steer.steer = 2 * dot( toRight, toTarget );
	// 2. mountains repel
	float2 probePos = spatial.pos + 8 * spatial.dir;
	for (int s = (int)Game::peaks.size(), i = 0; i < s; i++)
	{
		float peakMag = Game::peaks[i].z / 2;
		float2 toPeak = make_float2( Game::peaks[i].x, Game::peaks[i].y ) - probePos;
		float sqrDist = dot( toPeak, toPeak );
		if (sqrDist < sqrf( peakMag ))
			toPeak = normalize( toPeak ),
			steer.steer -= dot( toRight, toPeak ) * peakMag / sqrtf( sqrDist );
	}
	// 3. evade other tanks
	ActorList& nearby = Game::grid.FindNearbyTanks( this );
	for (int i = 0; i < nearby.count; i++)
	{
		Tank* tank = nearby.tank[i];
		float2 toActor = tank->spatial.pos - this->spatial.pos;
		float sqrDist = dot( toActor, toActor );
		if (sqrDist < 400 && dot( toActor, spatial.dir ) > 0.35f)
		{
			steer.steer -= (400 - sqrDist) * 0.02f * dot( toActor, toRight ) > 0 ? 1 : -1;
			break;
		}
	}
	// adjust heading and move
	movement.velocity = make_float2(0.5f);
	if (steer.steer < -0.2f) visual.frame = (visual.frame + 255 /* i.e. -1 */) & 255, spatial.dir = directions[visual.frame], movement.velocity = make_float2(0.35f);
	else if (steer.steer > 0.2f) visual.frame = (visual.frame + 1) & 255, spatial.dir = directions[visual.frame], make_float2(0.35f);
	else {
		// draw tank tracks, only when not turning
		float2 perp( -spatial.dir.y, spatial.dir.x );
		float2 trackPos1 = spatial.pos - 9 * spatial.dir + 4.5f * perp;
		float2 trackPos2 = spatial.pos - 9 * spatial.dir - 5.5f * perp;
		Game::map.bitmap->BlendBilerp( trackPos1.x, trackPos1.y, 0, 12 );
		Game::map.bitmap->BlendBilerp( trackPos2.x, trackPos2.y, 0, 12 );
	}
	spatial.pos += spatial.dir * movement.velocity;
}

// Bullet constructor
Bullet::Bullet(int2 p, uint f, int a)
{
	if (flash == nullptr)
	{
		// load static sprite data if not done yet
		flash = new Sprite("assets/flash.png");
		bullet = new Sprite("assets/bullet.png", make_int2(2, 2), make_int2(31, 31), 32, 256);
	}

	// create sprite instances based on the static sprite data
	// sprite = SpriteInstance(bullet);
	// frame = f;
	visual = {SpriteInstance(bullet), f};

	// set position and direction
	// pos = make_float2(p);
	// This assumes that a tank is constructed before any bullet is constructed.
	// dir = directions[f];
	spatial = {make_float2(p), directions[f]};
	// frameCounter = 0; // for keeping track of bullet lifetime
	lifetime = { 0 };
	// army = a;
	attack.army = a;
	flashSprite = SpriteInstance(flash);
}

// Bullet 'undraw': erase previously rendered pixels
void Bullet::Remove()
{
	// first visual.frame uses the 'flash' sprite; subsequent visual.frames the bullet sprite
	if (lifetime.age == 1)
		flashSprite.Remove();
	else
		visual.sprite.Remove();
}
	// TODO: Velocity != acceleration. I am not a physicist but I would encode this using an AccelerationComponent and an
	// AccelerationSystem. Basicaly it boils down needing to update

// Bullet behaviour
bool Bullet::Tick()
{
	TickPhysics();

	bool isAlive = TickLifetime();
	if (!isAlive) return isAlive;
	
	bool isOutOfBounds = TickBounds();
	if (isOutOfBounds) return isOutOfBounds;

	bool notHitAtTank = TickCollision();
	// When the bullet hits a tank, it dies.
	if (!notHitAtTank) return notHitAtTank;
	
	// stayin' alive
	return true;
}

void Bullet::TickPhysics()
{
	// update bullet position
	spatial.pos += spatial.dir * 8;
}

bool Bullet::TickLifetime()
{
	// destroy bullet if it travelled too long
	lifetime.age++;
	if (lifetime.age == 110)
	{
		Game::actorPool.push_back( new SpriteExplosion( this ) );
		return false;
	}
	return true;
}

bool Bullet::TickBounds()
{
	// destroy bullet if it leaves the map
	return spatial.pos.x < 0 || spatial.pos.y < 0 || spatial.pos.x > Game::map.width || spatial.pos.y > Game::map.height;
}

bool Bullet::TickCollision()
{
	// check if the bullet hit a tank
	ActorList& tanks = Game::grid.FindNearbyTanks( spatial.pos, 0 );
	for (int s = (int)tanks.count, i = 0; i < s; i++)
	{
		Tank* tank = tanks.tank[i]; // a tank, thankfully
		if (tank->attack.army == this->attack.army) continue; // no friendly fire. Disable for madness.
		float dist = length( this->spatial.pos - tank->spatial.pos );
		if (dist < 10)
		{
			tank->collision.hit_by_bullet = true; // tank will need to draw it's own conclusion
			return false; // bees die from stinging. Disable for rail gun.
		}
	}
	return true;
}

// Bullet Draw
void Bullet::Draw()
{
	// first visual.frame uses the 'flash' sprite; subsequent visual.frames the bullet sprite
	if (lifetime.age == 1 || lifetime.age == 159)
		flashSprite.Draw( Map::bitmap, spatial.pos, 0 );
	else
		visual.sprite.Draw( Map::bitmap, spatial.pos, visual.frame );
}

// ParticleExplosion constructor
ParticleExplosion::ParticleExplosion(Tank* tank)
{
	// read the pixels from the sprite of the specified tank
	const Sprite* sprite = tank->visual.sprite.sprite;
	const uint size = sprite->frameSize;
	const uint stride = sprite->frameSize * sprite->frameCount;
	const uint* src = sprite->pixels + tank->visual.frame * size;
	for (uint y = 0; y < size; y++) for (uint x = 0; x < size; x++)
	{
		const uint pixel = src[x + y * stride];
		const uint alpha = pixel >> 24;
		if (alpha > 64) for (int i = 0; i < 2; i++) // twice for a denser cloud
		{
			color.push_back( pixel & 0xffffff );
			const float fx = tank->spatial.pos.x - size * 0.5f + x;
			const float fy = tank->spatial.pos.y - size * 0.5f + y;
			pos.push_back( make_float2( fx, fy ) );
			dir.push_back( make_float2( 0, 0 ) );
		}
	}
}

// ParticleExplosion Draw
void ParticleExplosion::Draw()
{
	// create space to back up the map pixels we are about to modify
	if (!backup) backup = new uint[(int)pos.size() * 4];
	// draw the particles, with bilinear interpolation for smooth movement
	for (int s = (int)pos.size(), i = 0; i < s; i++)
	{
		int2 intPos = make_int2( pos[i] );
		// backup 2x2 pixels
		backup[i * 4 + 0] = Game::map.bitmap->Read( intPos.x, intPos.y );
		backup[i * 4 + 1] = Game::map.bitmap->Read( intPos.x + 1, intPos.y );
		backup[i * 4 + 2] = Game::map.bitmap->Read( intPos.x, intPos.y + 1 );
		backup[i * 4 + 3] = Game::map.bitmap->Read( intPos.x + 1, intPos.y + 1 );
		// draw one particle with bilerp, affecting four pixels
		Game::map.bitmap->BlendBilerp( pos[i].x, pos[i].y, color[i], fade );
	}
}

// ParticleExplosion behaviour
bool ParticleExplosion::Tick()
{
	for (int s = (int)pos.size(), i = 0; i < s; i++)
	{
		// move by adding particle speed stored in dir
		pos[i] += dir[i];
		// adjust dir randomly
		dir[i] -= make_float2( RandomFloat() * 0.05f + 0.02f, RandomFloat() * 0.02f - 0.01f );
	}
	// fadeout and kill actor when completely faded out
	if (fade-- == 0) return false; else return true;
}

// ParticleExplosion 'undraw'
void ParticleExplosion::Remove()
{
	// restore map pixels that we changed, in reverse order (this is important)
	for (int s = (int)pos.size(), i = s - 1; i >= 0; i--)
	{
		int2 intPos = make_int2( pos[i] );
		Game::map.bitmap->Plot( intPos.x, intPos.y, backup[i * 4 + 0] );
		Game::map.bitmap->Plot( intPos.x + 1, intPos.y, backup[i * 4 + 1] );
		Game::map.bitmap->Plot( intPos.x, intPos.y + 1, backup[i * 4 + 2] );
		Game::map.bitmap->Plot( intPos.x + 1, intPos.y + 1, backup[i * 4 + 3] );
	}
}

// SpriteExplosion constructor
SpriteExplosion::SpriteExplosion(Bullet* bullet)
{
	// load the static sprite data if it doesn't exist yet
	if (!anim) anim = new Sprite("assets/explosion1.png", 16);
	// set member variables
	visual =
	{
		// sprite = SpriteInstance( anim );
		SpriteInstance(anim),
		// frame = 0
		0
	};
	spatial =
	{
		// pos = bullet->physical.pos;
		bullet->spatial.pos
	};
}

// Particle constructor
Particle::Particle( Sprite* s, int2 p, uint c, uint d )
{
	physical.pos = make_float2( p );
	physical.dir = make_float2( -1 - RandomFloat() * 4, 0 );
	// frameChange = d;
	animate.frame_change = d;
	// sprite = SpriteInstance( s );
	visual.sprite = SpriteInstance(s);
}

// Particle behaviour
void Particle::Tick()
{
	TickPhysical();

	TickBounds();

	TickPeaksCollision();

	TickAnimate();
}

void Particle::TickPhysical()
{
	physical.pos += physical.dir;
	physical.dir.y *= 0.95f;
}

void Particle::TickBounds()
{
	if (physical.pos.x < 0)
	{
		physical.pos.x = (float)(Game::map.bitmap->width - 1);
		physical.pos.y = (float)(RandomUInt() % Game::map.bitmap->height);
		physical.dir = make_float2( -1 - RandomFloat() * 2, 0 );
	}
}

void Particle::TickPeaksCollision()
{
	for (int s = (int)Game::peaks.size(), i = 0; i < s; i++)
	{
		float2 toPeak = make_float2( Game::peaks[i].x, Game::peaks[i].y ) - physical.pos;
		float g = Game::peaks[i].z * 0.02f / sqrtf( dot( toPeak, toPeak ) );
		toPeak = normalize( toPeak );
		physical.dir.y -= toPeak.y * g;
	}
	physical.dir.y += RandomFloat() * 0.05f - 0.025f;
}

void Particle::TickAnimate()
{
	visual.frame = (visual.frame + animate.frame_change + 256) & 255;
}

