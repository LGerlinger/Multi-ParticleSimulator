#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include<fstream>

#include <stdlib.h>
#include <malloc.h>

#include "Vector2D.hpp"

#define PARTICLE_TEXTURE_PATH "disk256x256.png" // la racine étant là où sont les fichiers cpp (probablement là où est fait le make)

/*
// Good paramters for class Particles
struct world_t {
	double deltaT = 1.f / 500;

	double distanceCoef = 0.2;

	double gravity = 100;
	double chargeForce = 50000;
	double repelForce = 100000;
	double friction = 0.4;

	double fusionDistance = 0.4;

	double radius = 500;

	uint64_t time = 0;
};
*/



// Good paramters for the other ParticleSimultor derived classes
struct world_t {
	double deltaT = 1.f / 300;

	double distanceCoef = 0.1;

	double gravity = 15;
	double chargeForce = 10000;
	double repelForce = 1000000;
	double friction = 0.3;

	double fusionDistance = 0.4;

	double radius = 500;

	uint64_t time = 0;
};




class SingleParticle {
public:
	SingleParticle();
	SingleParticle(double x, double y, double _mass, double _radius, double _charge);
	SingleParticle(const SingleParticle& toCopy);

	double mass;
	double radius;
	double charge;

	Vector2D position;
	Vector2D speed;
	Vector2D acceleration;
};

struct tile_t {
	SingleParticle** Plist;

	uint16_t nbOfParticle = 0;
	double mass = 0;
	double charge = 0;
};