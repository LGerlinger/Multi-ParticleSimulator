#include "Particles2DArray.hpp"

#define PARTICLES_PER_TILE 20
#define NB_BODIES 5000

Particles2DArray::Particles2DArray(uint16_t _tileX, uint16_t _tileY, uint16_t _tileSize) {

	nbParticlesLeft = NB_BODIES;

	nTileX = _tileX;
	nTileY = _tileY;
	tileSize = _tileSize;
	sizeX = _tileSize * _tileX;
	sizeY = _tileSize * _tileY;

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


	// initialisation of each particle
	m_particles = (SingleParticle*)malloc(NB_BODIES * sizeof(SingleParticle));
	double max = 0;
	for (uint32_t p = 0; p < NB_BODIES; p++) {
		m_particles[p] = SingleParticle(rand() % sizeX, rand() % sizeY, rand() % 500 + 1, 0, rand() % 7 - 3);
		m_particles[p].radius = pow(m_particles[p].mass, 1 / 3.f);
		if (max < m_particles[p].radius) max = m_particles[p].radius;

		//m_particles[p].speed = sf::Vector2<double>(rand() % 31 - 15, rand() % 31 - 15);
	}

	// Normalizing particles radius to half a tile
	for (uint32_t p = 0; p < NB_BODIES; p++) {
		m_particles[p].radius *= tileSize / (2*max);
	}

	// Creating tiles and filling them
	tiles = (tile_t*)malloc(nTileX  * nTileY * sizeof(tile_t));

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


void Particles2DArray::solver() {
	world.time++;

	// ---- Calculating forces and interaction for particles alone ----
	Vector2D gravityAngle;
	double frictionCoef;
	for (uint32_t p1 = 0; p1 < NB_BODIES; p1++) {
		gravityAngle.redef(1000 - m_particles[p1].position.x,
							500 - m_particles[p1].position.y);
		gravityAngle *= (double)400 * pow(1 / gravityAngle.norme, 2);
		m_particles[p1].acceleration = gravityAngle * ((double)gravity * m_particles[p1].mass);


		frictionCoef = world.friction * m_particles[p1].speed.norme;
		m_particles[p1].acceleration -= m_particles[p1].speed * frictionCoef;


		//m_particles[p1].acceleration.x += 5000 * cos(world.time * 0.001);
		m_particles[p1].acceleration.y += gravity * m_particles[p1].mass;

		// Enforcing world borders
		if (m_particles[p1].radius > m_particles[p1].position.y) { // touching up border
			m_particles[p1].acceleration.y += bordersElasticity * (m_particles[p1].radius - m_particles[p1].position.y);
			m_particles[p1].acceleration -= (m_particles[p1].speed) * bordersFriction;
		}
		if (sizeY - m_particles[p1].radius < m_particles[p1].position.y) { // touching down border
			m_particles[p1].acceleration.y += bordersElasticity * (sizeY - m_particles[p1].radius - m_particles[p1].position.y);
			m_particles[p1].acceleration -= (m_particles[p1].speed) * bordersFriction;
		}
		if (m_particles[p1].radius > m_particles[p1].position.x) { // touching left border
			m_particles[p1].acceleration.x += bordersElasticity * (m_particles[p1].radius - m_particles[p1].position.x);
			m_particles[p1].acceleration -= (m_particles[p1].speed) * bordersFriction;
		}
		if (sizeX - m_particles[p1].radius < m_particles[p1].position.x) { // touching right border
			m_particles[p1].acceleration.x += bordersElasticity * (sizeX - m_particles[p1].radius - m_particles[p1].position.x);
			m_particles[p1].acceleration -= (m_particles[p1].speed) * bordersFriction;
		}

		m_particles[p1].acceleration.calcNorm();
	}

	// ---- Calculating forces between particles ----
	double repelCoef;
	Vector2D direction;
	Vector2D relativeSpeed;
	double distance;
	double totalMass;

	for (uint32_t y = 0; y < nTileY; y++) {
		for (uint32_t x = 0; x < nTileX; x++) {
			for (uint16_t particle1 = 0; particle1 < tiles[nTileX * y + x].nbOfParticle; particle1++) {
				SingleParticle& p1 = *tiles[nTileX * y + x].Plist[particle1];
				
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
							for (uint16_t particle2 = (particle1+1) * (dx * dx + dy == 0); particle2 < tiles[nTileX * (y + dy) + x + dx].nbOfParticle; particle2++) {
								SingleParticle& p2 = *tiles[nTileX * (y + dy) + x + dx].Plist[particle2];

								// the two particles interact (repel and friction)
								direction = p2.position - p1.position;
								distance = direction.norme;
								if (distance == 0) {
									direction.redef(rand() + 1, rand() + 1);
									distance = direction.norme;
								}
								if (distance < p1.radius + p2.radius) {
									// repel
									repelCoef = world.repelForce * (distance - p1.radius - p2.radius);
									direction *= repelCoef / distance;

									// friction
									totalMass = p1.mass + p2.mass;
									relativeSpeed = (p2.speed - p1.speed) * contactFriction;

									p1.acceleration += direction + relativeSpeed * (p2.mass / totalMass);
									p2.acceleration -= direction - relativeSpeed * (p1.mass / totalMass);
								}
							}
						}
					}
				}
			}
		}
	}
	

	// removing all SingleParticle pointers from tiles' lists
	for (uint32_t t = 0; t < nTileX * nTileY; t++) {
		memset(tiles[t].Plist, 0, tiles[t].nbOfParticle * sizeof(SingleParticle*));
		tiles[t].nbOfParticle = 0;
	}

	// ---- Updating speed and position for each particle ----
	for (uint32_t p1 = 0; p1 < NB_BODIES; p1++) {
		m_particles[p1].acceleration /= m_particles[p1].mass;
		/*if (isnan(m_particles[p1].acceleration.x) || m_particles[p1].mass == 0) {
			std::cout << p1 << " just naned" << std::endl;
		}*/
		m_particles[p1].speed += (m_particles[p1].acceleration * world.deltaT);
		if (m_particles[p1].speed.norme > 300000000) {
			m_particles[p1].speed *= (300000000 / m_particles[p1].speed.norme);
		}
		m_particles[p1].position += (m_particles[p1].speed * world.deltaT);

		// Putting the particle in its rightful place (tile)
		if (0 < m_particles[p1].position.x && m_particles[p1].position.x < sizeX && 0 < m_particles[p1].position.y && m_particles[p1].position.y < sizeY) {
			tile_t& tile = tiles[nTileX * ((uint32_t)m_particles[p1].position.y / tileSize) + (uint32_t)m_particles[p1].position.x / tileSize];
			tile.Plist[tile.nbOfParticle] = &m_particles[p1];
			tile.nbOfParticle++;
		}
	}
}


void Particles2DArray::UpVertices() {
	double norm, max=0;
	uint8_t r=255, g=0, b=0, a=255;
	for (uint32_t p = 0; p < NB_BODIES; p++) {
		norm = m_particles[p].speed.norme;
		if (max < norm) max = norm;
		r = norm /1 < 255 ? norm /1 : 255;
		g = norm /6 < 255 ? norm /6 : 255;
		b = norm /6 < 255 ? norm /6 : 255;
		a = norm /1 < 255 ? norm /1 : 255;


		m_vertices[4 * p].position.x = (float)(m_particles[p].position.x - 2*m_particles[p].radius);
		m_vertices[4 * p].position.y = (float)(m_particles[p].position.y - 2* m_particles[p].radius);
		m_vertices[4 * p].color.r = r;
		m_vertices[4 * p].color.g = g;
		m_vertices[4 * p].color.b = b;
		m_vertices[4 * p].color.a = a;

		m_vertices[4 * p + 1].position.x = (float)(m_particles[p].position.x + 2* m_particles[p].radius);
		m_vertices[4 * p + 1].position.y = (float)(m_particles[p].position.y - 2* m_particles[p].radius);
		m_vertices[4 * p + 1].color.r = r;
		m_vertices[4 * p + 1].color.g = g;
		m_vertices[4 * p + 1].color.b = b;
		m_vertices[4 * p + 1].color.a = a;

		m_vertices[4 * p + 2].position.x = (float)(m_particles[p].position.x + 2* m_particles[p].radius);
		m_vertices[4 * p + 2].position.y = (float)(m_particles[p].position.y + 2* m_particles[p].radius);
		m_vertices[4 * p + 2].color.r = r;
		m_vertices[4 * p + 2].color.g = g;
		m_vertices[4 * p + 2].color.b = b;
		m_vertices[4 * p + 2].color.a = a;

		m_vertices[4 * p + 3].position.x = (float)(m_particles[p].position.x - 2* m_particles[p].radius);
		m_vertices[4 * p + 3].position.y = (float)(m_particles[p].position.y + 2* m_particles[p].radius);
		m_vertices[4 * p + 3].color.r = r;
		m_vertices[4 * p + 3].color.g = g;
		m_vertices[4 * p + 3].color.b = b;
		m_vertices[4 * p + 3].color.a = a;
	}
}
