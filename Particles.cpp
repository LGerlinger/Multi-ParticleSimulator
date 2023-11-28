#include "Particles.hpp"

#define NB_BODIES 200

Particles::Particles() {
	nbParticlesLeft = NB_BODIES;

	// loading particle texture
	if (!m_texture.loadFromFile(PARTICLE_TEXTURE_PATH)) {
		std::cout << "Particles::Particles() : failed loading texture from file " << PARTICLE_TEXTURE_PATH << "\n\tConstruction aborted (no extra memory allocated)" << std::endl;
		return;
	}

	// dimensioning the vertexArray
	m_vertices.setPrimitiveType(sf::Quads);
	m_vertices.resize(NB_BODIES * 4);

	// adding the particle texture to each vertex
	m_state.texture = &m_texture;
	sf::Vector2u textureSize = m_texture.getSize();

	sf::Vector2f upLeft = sf::Vector2f(0.f, 0.f);
	sf::Vector2f upRight = sf::Vector2f(0.f, (float)textureSize.y);
	sf::Vector2f downRight = sf::Vector2f((float)textureSize.x, (float)textureSize.y);
	sf::Vector2f downLeft= sf::Vector2f((float)textureSize.x, 0.f);

	for (uint32_t p = 0; p < NB_BODIES; p++) {
		m_vertices[4 * p].texCoords = upLeft;
		m_vertices[4 * p + 1].texCoords = upRight;
		m_vertices[4 * p + 2].texCoords = downRight;
		m_vertices[4 * p + 3].texCoords = downLeft;
	}


	m_particles = (SingleParticle*)malloc(NB_BODIES * sizeof(SingleParticle));
	// initialisation of each particle
	for (uint32_t p=0; p < NB_BODIES; p++) {
		m_particles[p] = SingleParticle(rand()%1000, rand() % 1000, rand()%50+1, 0, (rand() % (2*(uint8_t)maxCharge+1) - maxCharge));
		m_particles[p].radius = 3*pow(m_particles[p].mass, 1 / 3.f);
		putChargeColor(p);
		//m_particles[p].speed = sf::Vector2<double>(rand() % 31 - 15, rand() % 31 - 15);
	}
}


void Particles::solver() {
	world.time++;

	// ---- Calculating forces and interaction for particles alone ----
	Vector2D direction;
	Vector2D speed;
	double distance;
	double frictionCoef;
	for (uint32_t p1 = 0; p1 < nbParticlesLeft; p1++) {
		frictionCoef = -world.friction * m_particles[p1].speed.norme;
		m_particles[p1].acceleration = m_particles[p1].speed * frictionCoef;


		// Enforcing world borders
		direction.redef(world.radius - m_particles[p1].position.x,// Radial vector
						world.radius - m_particles[p1].position.y);
		distance = direction.norme;
		direction /= direction.norme; // normalazing

		speed = m_particles[p1].speed;
		double dotProduct = Vector2D::dot(speed, direction);
		if (distance > world.radius && dotProduct < 0) { // If outside the borders and leaving
			// Reflect speed normaly to radial vector
			m_particles[p1].speed.x = speed.x - 2*direction.x * dotProduct;
			m_particles[p1].speed.y = speed.y - 2*direction.y * dotProduct;
		}
	}


	// ---- Calculating forces between particles ----
	double massCoef;
	double chargeCoef;
	double repelCoef;
	double total_mass;

	for (uint32_t p1=0; p1 < nbParticlesLeft; p1++) {
		for (uint32_t p2=p1+1; p2 < nbParticlesLeft; p2++) {
			/*if (p2 == 165 && world.time == 2914 && p1 == 164) {
				std::cout << "Daworldo" << std::endl;
				if (isnan(m_vertices[p2].position.x)) {
					std::cout << "Daworldo" << std::endl;
				}
			}*/

			direction.redef(m_particles[p2].position.x - m_particles[p1].position.x,
							m_particles[p2].position.y - m_particles[p1].position.y);
			distance = direction.norme;
			direction /= distance; // normalisation
			distance = distance * world.distanceCoef;

			if (distance < world.fusionDistance) { // Both particle fusion
			//if (false) {
				//std::cout << "fusion " << p1 << " " << p2 << std::endl;
				
				// p1 is the child of the fusion
				total_mass = m_particles[p1].mass + m_particles[p2].mass;

				m_particles[p1].speed.redef((m_particles[p1].speed.x * m_particles[p1].mass + m_particles[p2].speed.x * m_particles[p2].mass) / total_mass,
					(m_particles[p1].speed.y * m_particles[p1].mass + m_particles[p2].speed.y * m_particles[p2].mass) / total_mass);

				m_particles[p1].mass = total_mass;
				m_particles[p1].charge = m_particles[p1].charge + m_particles[p2].charge;
				m_particles[p1].radius = 3*pow((m_particles[p1].mass), 1/3.f);
				putChargeColor(p1);

				// p2 is dispatched
				m_particles[p2].mass = 0.001;
				m_particles[p2].charge = 0;
				m_particles[p2].radius = 0;
				m_particles[p2].position.redef(10000 + rand(), 10000 + rand());
				m_particles[p2].speed.redef(0, 0);

				// Exchanging last particle with p2 to get rid of it
				destroyParticle(p2);
				nbParticlesLeft--;
				p2--;
			}

			else  {
				if (distance / world.distanceCoef < m_particles[p1].radius + m_particles[p2].radius) { // testing if the two particules are visually overlapping to apply inter-particle friction

					// friction
					double totalMass = m_particles[p1].mass + m_particles[p2].mass;
					Vector2D relativeSpeed = (m_particles[p2].speed - m_particles[p1].speed) * (double)(3 * (m_particles[p1].radius + m_particles[p2].radius - distance / world.distanceCoef));

					m_particles[p1].acceleration += relativeSpeed * (m_particles[p2].mass / totalMass);
					m_particles[p2].acceleration -= relativeSpeed * (m_particles[p1].mass / totalMass);
				}

				distance += 0.01f;
				massCoef = world.gravity * (m_particles[p1].mass * m_particles[p2].mass) * (double)pow(2 * atan(distance) / distance, 2);
				m_particles[p1].acceleration += (direction * massCoef);
				m_particles[p2].acceleration -= (direction * massCoef);

				chargeCoef = world.chargeForce * (m_particles[p1].charge * m_particles[p2].charge) * (double)pow(2 * atan(distance) / distance, 4);
				m_particles[p1].acceleration -= (direction * chargeCoef);
				m_particles[p2].acceleration += (direction * chargeCoef);

				repelCoef = world.repelForce * (m_particles[p1].radius + m_particles[p2].radius) * (double)pow(2 * atan(distance) / distance, 7);
				m_particles[p1].acceleration -= (direction * repelCoef);
				m_particles[p2].acceleration += (direction * repelCoef);
			}
		}
	}


	// ---- Updating speed and position for each particle ----
	for (uint32_t p1 = 0; p1 < NB_BODIES; p1++) {
		m_particles[p1].acceleration /= m_particles[p1].mass;
		m_particles[p1].speed += (m_particles[p1].acceleration * world.deltaT);
		if (m_particles[p1].speed.norme > 300000000) {
			m_particles[p1].speed *= (300000000 / m_particles[p1].speed.norme);
		}
		m_particles[p1].position += (m_particles[p1].speed * world.deltaT);
		if (isnan(m_particles[p1].position.x) || isnan(m_particles[p1].position.y)) {
			m_particles[p1].position.redef(0, 0);
			m_particles[p1].speed.redef(0, 0);
			m_particles[p1].acceleration.redef(0, 0);
		}
	}
}


inline void Particles::putChargeColor(uint32_t p) {
	uint8_t r = 255, g = 255, b = 255, a = 128;
	if (m_particles[p].charge > 0) {
		r = 255;

		if (m_particles[p].charge > maxCharge) {
			g = 0;
			b = 0;
		}
		else {
			g = ((float)(maxCharge - m_particles[p].charge) / maxCharge) * 255;
			b = ((float)(maxCharge - m_particles[p].charge) / maxCharge) * 255;
		}
	}
	else if (m_particles[p].charge == 0) {
		r = 255;
		g = 255;
		b = 255;
	}
	else {
		if (m_particles[p].charge < -maxCharge) {
			r = 0;
			g = 0;
		}
		else {
			r = ((float)(maxCharge + m_particles[p].charge) / maxCharge) * 255;
			g = ((float)(maxCharge + m_particles[p].charge) / maxCharge) * 255;
		}
		b = 255;
	}

	m_vertices[4 * p].color.r = r;
	m_vertices[4 * p].color.g = g;
	m_vertices[4 * p].color.b = b;
	m_vertices[4 * p].color.a = a;

	m_vertices[4 * p + 1].color.r = r;
	m_vertices[4 * p + 1].color.g = g;
	m_vertices[4 * p + 1].color.b = b;
	m_vertices[4 * p + 1].color.a = a;

	m_vertices[4 * p + 2].color.r = r;
	m_vertices[4 * p + 2].color.g = g;
	m_vertices[4 * p + 2].color.b = b;
	m_vertices[4 * p + 2].color.a = a;

	m_vertices[4 * p + 3].color.r = r;
	m_vertices[4 * p + 3].color.g = g;
	m_vertices[4 * p + 3].color.b = b;
	m_vertices[4 * p + 3].color.a = a;
}
