#pragma once
#include "structs.h"

namespace Tmpl8
{

class MyApp;

class Actor
{
public:
	enum { TANK = 0, BULLET, FLAG, PARTICLE_EXPLOSION, SPRITE_EXPLOSION  };
	virtual void Remove() { visual.sprite.Remove(); }
	virtual bool Tick() = 0;
	virtual uint GetType() = 0;
	virtual void Draw()
	{
		visual.sprite.Draw(Map::bitmap, physical.pos, visual.frame);

	}
	static inline float2* directions;


	VisualComponent visual;
	PhysicalComponent physical;
};

class Tank : public Actor
{
public:
	Tank( Sprite* s, int2 p, int2 t, int f, int a );
	bool Tick();
	uint GetType() { return Actor::TANK; }

	AttackComponent attack;
	CollisionComponent collision;
private:
	bool TickCollision();
	void TickAttack();
	void TickPhysics();
};

class Bullet : public Actor
{
public:
	Bullet( int2 p, int f, int a );
	void Remove();
	bool Tick();
	void Draw();
	uint GetType() { return Actor::BULLET; }
	SpriteInstance flashSprite;
	int frameCounter, army;
	static inline Sprite* flash = 0, *bullet = 0;
};

class ParticleExplosion : public Actor
{
public:
	ParticleExplosion() = default;
	ParticleExplosion( Tank* tank );
	~ParticleExplosion() { delete backup; }
	void Remove();
	bool Tick();
	void Draw();
	uint GetType() { return Actor::PARTICLE_EXPLOSION; }
	vector<float2> pos;
	vector<float2> dir;
	vector<uint> color;
	uint* backup = 0;
	uint fade = 255;
};

class SpriteExplosion : public Actor
{
public:
	SpriteExplosion() = default;
	SpriteExplosion( Bullet* bullet );
	bool Tick() { if (++visual.frame == 16) return false; return true; }
	void Draw() { visual.sprite.DrawAdditive( Map::bitmap, physical.pos, visual.frame - 1 ); }
	uint GetType() { return Actor::SPRITE_EXPLOSION; }
	static inline Sprite* anim = 0;
};

class Particle
{
public:
	Particle() = default;
	Particle( Sprite* s, int2 p, uint c, uint d );
	void Remove() { visual.sprite.Remove(); }
	void Tick();
	void Draw() { visual.sprite.Draw( Map::bitmap, physical.pos, visual.frame ); }

	VisualComponent visual;
	PhysicalComponent physical;
	AnimateComponent animate;
};

} // namespace Tmpl8