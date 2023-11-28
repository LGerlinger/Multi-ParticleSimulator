#pragma once
#include "ParticleSimulator.hpp"

class PPC : public ParticleSimulator
{ // Adds a world grid and redefines the solver method
public:
	PPC(uint16_t _tileX, uint16_t _tileY, uint16_t _tileSize);

	virtual void solver();

	virtual void UpVertices();
	inline void putChargeColor(uint32_t p);

	tile_t* tiles;

	uint32_t sizeX; // horizontal size of the world
	uint32_t sizeY; // vertical size of the world
	uint16_t nTileX; // number of tiles on the x axis
	uint16_t nTileY; // number of tiles on the y axis
	uint16_t tileSize; // size of a single tile

	float gravity = 200;
	short maxCharge = 4;
	uint8_t nFrameRepel = 8;

	double minSpeed;
	double maxSpeed;

	Vector2D center;

	world_t world; // Defines the constants of this world
};