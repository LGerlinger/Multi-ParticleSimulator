#include "ParticleSimulator.hpp"


void ParticleSimulator::UpVertices() {
	for (uint32_t p = 0; p < nbParticlesLeft; p++) {
		m_vertices[4 * p].position.x = (float)(m_particles[p].position.x - m_particles[p].radius);
		m_vertices[4 * p].position.y = (float)(m_particles[p].position.y - m_particles[p].radius);

		m_vertices[4 * p + 1].position.x = (float)(m_particles[p].position.x + m_particles[p].radius);
		m_vertices[4 * p + 1].position.y = (float)(m_particles[p].position.y - m_particles[p].radius);

		m_vertices[4 * p + 2].position.x = (float)(m_particles[p].position.x + m_particles[p].radius);
		m_vertices[4 * p + 2].position.y = (float)(m_particles[p].position.y + m_particles[p].radius);

		m_vertices[4 * p + 3].position.x = (float)(m_particles[p].position.x - m_particles[p].radius);
		m_vertices[4 * p + 3].position.y = (float)(m_particles[p].position.y + m_particles[p].radius);
	}
}

void ParticleSimulator::destroyParticle(uint32_t p) {
	SingleParticle temp = m_particles[p];
	m_particles[p] = m_particles[nbParticlesLeft-1];
	m_particles[nbParticlesLeft-1] = temp;

	for (uint8_t i = 0; i < 4; i++) {
		m_vertices[4 * p + i] = m_vertices[4 * (nbParticlesLeft - 1) + i];
	}
	m_vertices.resize(4 * (nbParticlesLeft-1));
}

void ParticleSimulator::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	// on applique la transformation
	//states.transform *= getTransform();

	// on applique la texture du tileset
	states.texture = &m_texture;

	// et on dessine enfin le tableau de vertex
	target.draw(m_vertices, states);
}


void ParticleSimulator::openFileForSaving() {
	saveFile.open("saves/m_particles.bin", std::ios::out | std::ios::binary | std::ios::trunc);
	if (!saveFile.failbit) {
		std::cout << "Failed opening file : saves/m_particles.bin" << std::endl;
	}
	saveFile.close();

	saveFile = std::ofstream("saves/m_particles.bin", std::ios::out | std::ios::binary | std::ios::app);

	saveBuffer = (double*)malloc(2 * nbParticlesLeft * sizeof(double));
}

void ParticleSimulator::closeFileForSaving() {
	delete saveBuffer;
	saveFile.close();
}

// You must have called openFileForLoading method for this object before calling this method (savePositions)
void ParticleSimulator::savePositions() {
	for (uint32_t p = 0; p < nbParticlesLeft; p++) {
		saveBuffer[2 * p] = m_particles[p].position.x;
		saveBuffer[2 * p +1] = m_particles[p].position.y;
	}

	saveFile.write((char*)&nbParticlesLeft, sizeof(uint32_t));
	saveFile.write((char*)saveBuffer, 2 * nbParticlesLeft * sizeof(double));
}



void ParticleSimulator::openFileForLoading() {
	loadFile.open("saves/m_particles.bin", std::ifstream::in | std::ifstream::binary);
	if (!loadFile.is_open()) {
		std::cout << "Failed opening file : saves/m_particles.bin" << std::endl;
	}
	loadFile.seekg(0, loadFile.beg);

	loadBuffer = (double*)malloc(2 * nbParticlesLeft * sizeof(double));
}

void ParticleSimulator::closeFileForLoading() {
	delete loadBuffer;
	loadFile.close();
}

// You must have called openFileForLoading method for this object before calling this method (savePositions)
void ParticleSimulator::loadPositions() {
	loadFile.read((char*)&nbParticlesLeft, sizeof(uint32_t));

	loadFile.read((char*)loadBuffer, 2 * nbParticlesLeft * sizeof(double));

	for (uint32_t p = 0; p < nbParticlesLeft; p++) {
		m_particles[p].position.x = loadBuffer[2 * p];
		m_particles[p].position.y = loadBuffer[2 * p +1];
	}
}