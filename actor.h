#pragma once
#include "structs.h"

namespace Tmpl8
{

class MyApp;

class Actor
{
public:
	enum { TANK = 0, BULLET, FLAG, PARTICLE_EXPLOSION, SPRITE_EXPLOSION  };
	virtual void Remove() {  }
	virtual bool Tick() = 0;
	virtual uint GetType() = 0;
	virtual void Draw() {}
	static inline float2* directions;
};

class Tank : public Actor
{
public:
	Tank(VisualComponent* visual, SpatialComponent* spatial, SteerComponent* steer, AttackComponent* attack,
	     CollisionComponent* collision):
		Actor(), visual(visual), spatial(spatial), steer(steer), attack(attack), collision(collision)
	{
	}

	void Remove() override {  }
	bool Tick() override;
	uint GetType() override { return Actor::TANK; }

	VisualComponent* visual;
	SpatialComponent* spatial;
	SteerComponent* steer;
	AttackComponent* attack;
	CollisionComponent* collision;

private:
	bool TickCollision();
	void TickAttack();
	void TickPhysics();
};

class Bullet : public Actor
{
public:
	Bullet( int2 p, uint f, int a );
	void Remove() override;
	bool Tick() override;
	void Draw() override;
	uint GetType() override { return Actor::BULLET; }
	// TODO: Our current component implementation doesn't allow for the same entity to have multiple sprites. I think we
	// need to move towards component managers as is described in
	// https://bitsquid.blogspot.com/2014/08/building-data-oriented-entity-system.html.
	SpriteInstance flashSprite;
	static inline Sprite* flash = 0, *bullet = 0;

	VisualComponent visual;
	SpatialComponent spatial;

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
	void Remove() override;
	bool Tick() override;
	void Draw() override;
	uint GetType() override { return Actor::PARTICLE_EXPLOSION; }
	vector<float2> pos;
	vector<float2> dir;
	vector<uint> color;
	uint* backup = 0;
	uint fade = 255;


	VisualComponent visual;
	SpatialComponent spatial;
};

class SpriteExplosion : public Actor
{
public:
	SpriteExplosion() = default;
	SpriteExplosion( Bullet* bullet );
	bool Tick() override { if (++visual.frame == 16) return false; return true; }
	void Draw() override { visual.sprite.DrawAdditive( Map::bitmap, spatial.pos, visual.frame - 1 ); }
	void Remove() override { visual.sprite.Remove(); }
	uint GetType() override { return Actor::SPRITE_EXPLOSION; }
	static inline Sprite* anim = 0;

	VisualComponent visual;
	SpatialComponent spatial;
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