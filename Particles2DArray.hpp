#pragma once
#include "ParticleSimulator.hpp"

// 90 FPS pour 5000 particles et un écran à 1900x1000


class Particles2DArray : public ParticleSimulator
{ // Adds a world grid and redefines the solver method
public:
	Particles2DArray(uint16_t _tileX, uint16_t _tileY, uint16_t _tileSize);

	virtual void solver();

	virtual void UpVertices();

	tile_t* tiles;

	uint32_t sizeX; // horizontal size of the world
	uint32_t sizeY; // vertical size of the world
	uint16_t nTileX; // number of tiles on the x axis
	uint16_t nTileY; // number of tiles on the y axis
	uint16_t tileSize; // size of a single tile

	double bordersElasticity = 200000;
	double bordersFriction = 0;
	float gravity = 200;
	double contactFriction = 10;

	world_t world; // Defines the constants of this world
};