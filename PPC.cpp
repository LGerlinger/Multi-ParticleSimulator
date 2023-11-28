#include "PPC.hpp"

#define PARTICLES_PER_TILE 40
#define NB_BODIES 5000

PPC::PPC(uint16_t _tileX, uint16_t _tileY, uint16_t _tileSize) {

	world.deltaT = 1/25.f;

	nbParticlesLeft = NB_BODIES;

	nTileX = _tileX;
	nTileY = _tileY;
	tileSize = _tileSize;
	sizeX = _tileSize * _tileX;
	sizeY = _tileSize * _tileY;

	center.redef(sizeX / 2, sizeY / 2);

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
		angle = (rand() % 3600) * 3.141592654 / 1800;
		radius = pow((double)(rand() % RAND_MAX) / RAND_MAX, 1/2.f) * 495;
		m_particles[p] = SingleParticle(center.x + radius*cos(angle), center.y + radius*sin(angle), rand() % 250 + 1, 0, rand() % (2*maxCharge+1) - maxCharge);
		m_particles[p].radius = pow(m_particles[p].mass, 1 / 3.f);
		if (max < m_particles[p].radius) max = m_particles[p].radius;
		if (min > m_particles[p].radius) min = m_particles[p].radius;

		putChargeColor(p);

		//m_particles[p].speed = sf::Vector2<double>(rand() % 31 - 15, rand() % 31 - 15);
	}
	min--;

	// Normalizing particles radius to half of a tile
	for (uint32_t p = 0; p < NB_BODIES; p++) {
		m_particles[p].radius = (m_particles[p].radius - min) * tileSize / (2 * (max-min));
	}

	// Creating tiles and filling them
	tiles = (tile_t*)malloc(nTileX * nTileY * sizeof(tile_t));

	for (uint64_t t = 0; t < nTileX * nTileY; t++) {
		tiles[t].Plist = (SingleParticle**)malloc(PARTICLES_PER_TILE * sizeof(SingleParticle*));
		tiles[t].nbOfParticle = 0;
	}
	tile_t& tile = tiles[0];

	for (uint32_t p = 0; p < NB_BODIES; p++) {
		tile = tiles[nTileX * ((uint32_t)m_particles[p].position.y / tileSize) + (uint32_t)m_particles[p].position.x / tileSize];
		tile.Plist[tile.nbOfParticle] = &m_particles[p];
		tile.nbOfParticle++;
	}
}


void PPC::solver() {
	world.time++;

	// ---- Calculating forces and interaction for particles alone ----
	Vector2D force;
	double temp;
	for (uint32_t p1 = 0; p1 < NB_BODIES; p1++) {
		force = center - m_particles[p1].position;

		m_particles[p1].acceleration = force * gravity * m_particles[p1].mass * pow(atan(force.norme) / force.norme, 2);

		temp = force.x;
		force.x = -force.y;
		force.y = temp;
		force *= pow(atan(force.norme) / force.norme, 4);
		force *= 1000000;
		m_particles[p1].acceleration += force;

		//m_particles[p1].acceleration.calcNorm();

		m_particles[p1].acceleration /= m_particles[p1].mass;
		m_particles[p1].speed += (m_particles[p1].acceleration * world.deltaT);
	}

	// ---- Calculating forces between particles ----
	
	for (uint32_t y = 0; y < nTileY; y++) {
		for (uint32_t x = 0; x < nTileX; x++) {
			for (uint16_t particle1 = 0; particle1 < tiles[nTileX * y + x].nbOfParticle; particle1++) {
				SingleParticle& sp1 = *tiles[nTileX * y + x].Plist[particle1];

				// cases : 
				// dx=0 && dy=0 then start after the particle p1
				// dx = 1 && dy = 0
				// dx = -1:1 && dy = 1

				// This is ugly but I also find it funny
				for (short dy = 0; dy < 2; dy++) {
					if (y + dy >= (short)nTileY)
						break;
					for (short dx = -dy; dx < 2; dx++) {
						if (0 <= x + dx && x + dx < nTileX) {
							for (uint16_t particle2 = (particle1 + 1) * (dx * dx + dy == 0); particle2 < tiles[nTileX * (y + dy) + x + dx].nbOfParticle; particle2++) {
								SingleParticle& sp2 = *tiles[nTileX * (y + dy) + x + dx].Plist[particle2];


								Vector2D relativeSpeed((sp1.speed.x - sp2.speed.x) * world.deltaT,
													   (sp1.speed.y - sp2.speed.y) * world.deltaT);

								double distX = sp2.position.x - sp1.position.x;
								double distY = sp2.position.y - sp1.position.y;

								double C = distX * distX + distY * distY - (sp1.radius + sp2.radius) * (sp1.radius + sp2.radius);

								if (C >= 0 && sqrt(C) < tileSize) {
									// Calculating charge force
									Vector2D chargeForce = sp2.position - sp1.position;
									chargeForce *= -1500 * sp1.charge * sp2.charge / chargeForce.norme;
									sp1.speed += chargeForce * (world.deltaT / sp1.mass / (distX * distX + distY * distY));
									sp2.speed -= chargeForce * (world.deltaT / sp2.mass / (distX * distX + distY * distY));
								}

								if (C < 0) { // Particles are overlapping
									Vector2D v2(sp1.position.x - sp2.position.x,
										sp1.position.y - sp2.position.y);
									v2 /= v2.norme;
									v2 *= (sp1.radius + sp2.radius - v2.norme) * nFrameRepel * world.deltaT;

									sp1.speed += v2 * (sp2.mass / (sp1.mass + sp2.mass));
									sp2.speed -= v2 * (sp1.mass / (sp1.mass + sp2.mass));

								}
								else {
									double A = relativeSpeed.norme * relativeSpeed.norme;
									double B = -2 * (relativeSpeed.x * distX + relativeSpeed.y * distY);

									double delta = B * B - 4 * A * C;

									if (delta > 0) { // sp1 and sp2 will enventually hit --- if delta=0 then there's only 1 intersection and we don't care
										float alpha1 = (-B + sqrt(delta)) / (2 * A);
										float alpha2 = (-B - sqrt(delta)) / (2 * A);

										alpha1 = alpha1 < alpha2 ? alpha1 : alpha2; // assuring alpha1 < alpha2

										if (0 <= alpha1 && alpha1 < 1) { // sp1 and sp2 are about to hit
											Vector2D v2(sp2.position.x - sp1.position.x - relativeSpeed.x * alpha1,
												sp2.position.y - sp1.position.y - relativeSpeed.y * alpha1);

											v2 /= v2.norme;
											v2 *= Vector2D::dot(relativeSpeed, v2);

											// this est repoussé selon la normale à l'intersection proportionnelement à la masse de l'autre
											sp1.speed -= v2 * (sp2.mass / (sp1.mass + sp2.mass)) / world.deltaT;
											sp2.speed += v2 * (sp1.mass / (sp1.mass + sp2.mass)) / world.deltaT;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// ---- Enforcing world borders ----
	
	//// Up
	uint16_t x, y=0;
	for (x = 0; x < nTileX; x++) {
		for (uint32_t p1 = 0; p1 < tiles[nTileX * y + x].nbOfParticle; p1++) {
			SingleParticle& sp1 = *tiles[nTileX * y + x].Plist[p1];
			if (sp1.position.y + sp1.speed.y * world.deltaT < sp1.radius) {
				sp1.speed.y = std::max((sp1.radius - sp1.position.y) / world.deltaT, sp1.speed.y);
				sp1.speed.calcNorm();
			}
		}
	}

	// Down
	y = nTileY-1;
	for (x = 0; x < nTileX; x++) {
		for (uint32_t p1 = 0; p1 < tiles[nTileX * y + x].nbOfParticle; p1++) {
			SingleParticle& sp1 = *tiles[nTileX * y + x].Plist[p1];
			if (sp1.position.y + sp1.speed.y * world.deltaT > sizeY - sp1.radius) {
				sp1.speed.y = std::min((sizeY - sp1.radius - sp1.position.y)/world.deltaT, sp1.speed.y);
				sp1.speed.calcNorm();
			}
		}
	}
	
	// left
	x = 0;
	for (y = 0; y < nTileY; y++) {
		for (uint32_t p1 = 0; p1 < tiles[nTileX * y + x].nbOfParticle; p1++) {
			SingleParticle& sp1 = *tiles[nTileX * y + x].Plist[p1];
			if (sp1.position.x + sp1.speed.x * world.deltaT < sp1.radius) {
				sp1.speed.x = std::max((sp1.radius - sp1.position.x) / world.deltaT, sp1.speed.x);
				sp1.speed.calcNorm();
			}
		}
	}

	// Right
	x = nTileX - 1;
	for (y = 0; y < nTileY; y++) {
		for (uint32_t p1 = 0; p1 < tiles[nTileX * y + x].nbOfParticle; p1++) {
			SingleParticle& sp1 = *tiles[nTileX * y + x].Plist[p1];
			if (sp1.position.x + sp1.speed.x * world.deltaT > sizeX - sp1.radius) {
				sp1.speed.x = std::min((sizeX - sp1.radius - sp1.position.x) / world.deltaT, sp1.speed.x);
				sp1.speed.calcNorm();
			}
		}
	}
	
	// ---- End of enforcing world borders ----


	// removing all SingleParticle pointers from tiles' lists
	for (uint32_t t = 0; t < nTileX * nTileY; t++) {
		memset(tiles[t].Plist, 0, tiles[t].nbOfParticle * sizeof(SingleParticle*));
		tiles[t].nbOfParticle = 0;
	}

	// ---- Updating position for each particle ----
	minSpeed = INFINITY;
	maxSpeed = 0;
	for (uint32_t p1 = 0; p1 < NB_BODIES; p1++) {
		minSpeed = minSpeed > m_particles[p1].speed.norme ? m_particles[p1].speed.norme : minSpeed;
		maxSpeed = maxSpeed < m_particles[p1].speed.norme ? m_particles[p1].speed.norme : maxSpeed;

		m_particles[p1].position += (m_particles[p1].speed * world.deltaT);

		// Putting the particle in its rightful place (tile)
		if (0 < m_particles[p1].position.x && m_particles[p1].position.x < sizeX && 0 < m_particles[p1].position.y && m_particles[p1].position.y < sizeY) {
			tile_t& tile = tiles[nTileX * ((uint32_t)m_particles[p1].position.y / tileSize) + (uint32_t)m_particles[p1].position.x / tileSize];
			tile.Plist[tile.nbOfParticle] = &m_particles[p1];
			tile.nbOfParticle++;
		}
	}
}

// maxcharge = 3
void PPC::UpVertices() {
	uint8_t r = 255, g = 255, b = 255, a = 255;
	for (uint32_t p = 0; p < NB_BODIES; p++) {
		//a = (m_particles[p].speed.norme - minSpeed) * 100 / (maxSpeed - minSpeed) + 155;
		
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

inline void PPC::putChargeColor(uint32_t p) {
	uint8_t r = 255, g = 255, b = 255;
	if (m_particles[p].charge > 0) {
		r = 255;
		g = ((float)(maxCharge - m_particles[p].charge) / maxCharge) * 255;
		b = ((float)(maxCharge - m_particles[p].charge) / maxCharge) * 255;
	}
	else if (m_particles[p].charge == 0) {
		r = 255;
		g = 255;
		b = 255;
	}
	else {
		r = ((float)(maxCharge + m_particles[p].charge) / maxCharge) * 255;
		g = ((float)(maxCharge + m_particles[p].charge) / maxCharge) * 255;
		b = 255;
	}

	m_vertices[4 * p].color.r = r;
	m_vertices[4 * p].color.g = g;
	m_vertices[4 * p].color.b = b;

	m_vertices[4 * p + 1].color.r = r;
	m_vertices[4 * p + 1].color.g = g;
	m_vertices[4 * p + 1].color.b = b;

	m_vertices[4 * p + 2].color.r = r;
	m_vertices[4 * p + 2].color.g = g;
	m_vertices[4 * p + 2].color.b = b;

	m_vertices[4 * p + 3].color.r = r;
	m_vertices[4 * p + 3].color.g = g;
	m_vertices[4 * p + 3].color.b = b;
}
