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
		visual.sprite.Draw(Map::bitmap, spatial.pos, visual.frame);

	}
	static inline float2* directions;


	VisualComponent visual;
	SpatialComponent spatial;
};

class Tank : public Actor
{
public:
	Tank( Sprite* s, int2 p, int2 t, uint f, int a );
	Tank(const VisualComponent& visual, SpatialComponent spatial, TargetComponent target, AttackComponent attack, CollisionComponent collision );
	bool Tick();
	uint GetType() { return Actor::TANK; }

	TargetComponent target;
	MovementComponent movement;
	SteerComponent steer;
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
	Bullet( int2 p, uint f, int a );
	void Remove();
	bool Tick();
	void Draw();
	uint GetType() { return Actor::BULLET; }
	// TODO: Our current component implementation doesn't allow for the same entity to have multiple sprites. I think we
	// need to move towards component managers as is described in
	// https://bitsquid.blogspot.com/2014/08/building-data-oriented-entity-system.html.
	SpriteInstance flashSprite;
	static inline Sprite* flash = 0, *bullet = 0;

	// int frameCounter, army;
	AttackComponent attack;
	LifetimeComponent lifetime;
private:
	void TickPhysics();
	bool TickLifetime();
	bool TickBounds();
	bool TickCollision();
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
	void Draw() { visual.sprite.DrawAdditive( Map::bitmap, spatial.pos, visual.frame - 1 ); }
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
	SpatialComponent physical;
	AnimateComponent animate;
private:
	void TickPhysical();
	void TickBounds();
	void TickPeaksCollision();
	void TickAnimate();
};

} // namespace Tmpl8