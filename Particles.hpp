#pragma once
#include "ParticleSimulator.hpp"

// 70 FPS pour 200 particles et un écran à 1900x1000

class Particles : public ParticleSimulator
{
public:
	Particles();

	virtual void solver();

	inline void putChargeColor(uint32_t p);

	double maxCharge = 10;
};