#pragma once
#include "ParticleSimulator.hpp"

class NoInteraction : public ParticleSimulator
{ // this Simulator doesn't do any interaction between particles meaning it can handle many more
public:
	NoInteraction(uint32_t centerX, uint32_t centerY);

	virtual void solver();

	virtual void UpVertices();
	inline void putColor(uint32_t p);

	Vector2D center;

	double minSpeed;
	double maxSpeed;

	world_t world; // Defines the constants of this world
};