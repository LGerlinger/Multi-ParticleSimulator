#include "NoInteraction.hpp"

#define NB_BODIES 40000

NoInteraction::NoInteraction(uint32_t centerX, uint32_t centerY) {
	world.deltaT = 1 / 250.f;

	nbParticlesLeft = NB_BODIES;

	center.redef(centerX / 2, centerY / 2);

	// loading particle texture
	if (!m_texture.loadFromFile(PARTICLE_TEXTURE_PATH))
		return;
	// dimensioning the vertexArray
	m_vertices.setPrimitiveType(sf::Quads);
	m_vertices.resize(NB_BODIES * 4);

	// adding the particle texture to each vertex
	m_state.texture = &m_texture;
	sf::Vector2u textureSize = m_texture.getSize();

	sf::Vector2f upLeft = sf::Vector2f(0.f, 0.f);
	sf::Vector2f upRight = sf::Vector2f(0.f, (float)textureSize.y);
	sf::Vector2f downRight = sf::Vector2f((float)textureSize.x, (float)textureSize.y);
	sf::Vector2f downLeft = sf::Vector2f((float)textureSize.x, 0.f);

	for (uint32_t p = 0; p < NB_BODIES; p++) {
		m_vertices[4 * p].texCoords = upLeft;
		m_vertices[4 * p + 1].texCoords = upRight;
		m_vertices[4 * p + 2].texCoords = downRight;
		m_vertices[4 * p + 3].texCoords = downLeft;
	}


	// initialisation of each particle
	m_particles = (SingleParticle*)malloc(NB_BODIES * sizeof(SingleParticle));
	double max = 0, min = INFINITY;
	double angle, radius;
	for (uint32_t p = 0; p < NB_BODIES; p++) {
		//angle = (rand() % 3600) * 3.141592654 / 1800;
		angle = (rand() % 3600 + 0) * 3.141592654 / 1800;
		radius = pow((double)(rand() % RAND_MAX) / RAND_MAX, 1 / 2.f) * 495;
		m_particles[p] = SingleParticle(center.x + radius * cos(angle), center.y + radius * sin(angle), rand() % 250 + 1, 0, 0);
		m_particles[p].radius = pow(m_particles[p].mass, 1 / 3.f);
		if (max < m_particles[p].radius) max = m_particles[p].radius;
		if (min > m_particles[p].radius) min = m_particles[p].radius;

		putColor(p);

		//m_particles[p].speed = sf::Vector2<double>(rand() % 31 - 15, rand() % 31 - 15);
	}
	min--;

	// Normalizing particles radius to half of a tile
	for (uint32_t p = 0; p < NB_BODIES; p++) {
		m_particles[p].radius = (m_particles[p].radius - min) * 10 / (2 * (max - min));
	}
}

void NoInteraction::solver() {
	Vector2D v;
	for (uint32_t p = 0; p < nbParticlesLeft; p++) {
		v = center - m_particles[p].position;
		if (m_particles[p].position.y > center.y) {
			//if (m_particles[p].position.y > center.y) { // down right
			m_particles[p].acceleration.y = 2000 * (1/4.f * v.y) * pow(10 * atan(v.norme) / v.norme, 2);
			m_particles[p].acceleration.x = 2000 * (2 * v.x) * pow(10 * atan(v.norme) /v.norme, 2);
			//}
			//else { // up right

			//}
		}
		else {
			if (m_particles[p].position.x > center.x) { // down left
				m_particles[p].acceleration.y = -v.x;
				m_particles[p].acceleration.x = v.y;
			}
			else { // up left
				m_particles[p].acceleration.y = v.x;
				m_particles[p].acceleration.x = -v.y;
			}
		}
	}

	// ---- Updating position for each particle ----
	minSpeed = INFINITY;
	maxSpeed = 0;
	for (uint32_t p = 0; p < NB_BODIES; p++) {
		m_particles[p].speed += m_particles[p].acceleration * world.deltaT;
		minSpeed = minSpeed > m_particles[p].speed.norme ? m_particles[p].speed.norme : minSpeed;
		maxSpeed = maxSpeed < m_particles[p].speed.norme ? m_particles[p].speed.norme : maxSpeed;

		m_particles[p].position += (m_particles[p].speed * world.deltaT);
	}
}


void NoInteraction::UpVertices() {
	uint8_t a = 126;
	for (uint32_t p = 0; p < NB_BODIES; p++) {
		//a = 255 - (uint8_t)(m_particles[p].speed.norme - minSpeed) * 255 / (maxSpeed - minSpeed) + 0;

		m_vertices[4 * p].position.x = (float)(m_particles[p].position.x - m_particles[p].radius);
		m_vertices[4 * p].position.y = (float)(m_particles[p].position.y - m_particles[p].radius);
		m_vertices[4 * p].color.a = a;

		m_vertices[4 * p + 1].position.x = (float)(m_particles[p].position.x + m_particles[p].radius);
		m_vertices[4 * p + 1].position.y = (float)(m_particles[p].position.y - m_particles[p].radius);
		m_vertices[4 * p + 1].color.a = a;

		m_vertices[4 * p + 2].position.x = (float)(m_particles[p].position.x + m_particles[p].radius);
		m_vertices[4 * p + 2].position.y = (float)(m_particles[p].position.y + m_particles[p].radius);
		m_vertices[4 * p + 2].color.a = a;

		m_vertices[4 * p + 3].position.x = (float)(m_particles[p].position.x - m_particles[p].radius);
		m_vertices[4 * p + 3].position.y = (float)(m_particles[p].position.y + m_particles[p].radius);
		m_vertices[4 * p + 3].color.a = a;
	}
}

inline void NoInteraction::putColor(uint32_t p) {
	uint8_t r = rand()%255, g = rand() % 255, b = rand() % 255;

	m_vertices[4 * p].color.r = r;
	m_vertices[4 * p].color.g = g;
	m_vertices[4 * p].color.b = b;

	r = rand() % 255; g = rand() % 255; b = rand() % 255; 
	m_vertices[4 * p + 1].color.r = r;
	m_vertices[4 * p + 1].color.g = g;
	m_vertices[4 * p + 1].color.b = b;

	r = rand() % 255; g = rand() % 255; b = rand() % 255;
	m_vertices[4 * p + 2].color.r = r;
	m_vertices[4 * p + 2].color.g = g;
	m_vertices[4 * p + 2].color.b = b;

	r = rand() % 255; g = rand() % 255; b = rand() % 255;
	m_vertices[4 * p + 3].color.r = r;
	m_vertices[4 * p + 3].color.g = g;
	m_vertices[4 * p + 3].color.b = b;
}