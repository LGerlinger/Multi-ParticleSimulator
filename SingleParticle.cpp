#include "SingleParticle.hpp"

SingleParticle::SingleParticle() {
	mass = 0;
	radius = 0;
	charge = 0;
}

SingleParticle::SingleParticle(double x, double y, double _mass, double _radius, double _charge) {
	mass = _mass;
	radius = _radius;
	charge = _charge;

	position.redef(x, y);

	speed.x = 0;
	speed.y = 0;
	speed.norme = 0;

	acceleration.x = 0;
	acceleration.y = 0;
	acceleration.norme = 0;
}

SingleParticle::SingleParticle(const SingleParticle& toCopy) {
	*this = toCopy;
}