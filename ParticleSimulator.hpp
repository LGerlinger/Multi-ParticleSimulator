#pragma once
#include "SingleParticle.hpp"

class ParticleSimulator : public sf::Drawable, public sf::Transformable
{
public:
	virtual void solver() = 0;

	virtual void UpVertices();

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	void destroyParticle(uint32_t p);

	sf::VertexArray m_vertices;
	sf::Texture m_texture;
	sf::RenderStates m_state;
	SingleParticle* m_particles; // To allocate yourself // 2^32 -1 particles max

	world_t world; // Defines the constants of the world that will be used

	uint32_t nbParticlesLeft;

	void openFileForSaving();
	void closeFileForSaving();
	void savePositions();
	std::ofstream saveFile;
	double* saveBuffer;

	void openFileForLoading();
	void closeFileForLoading();
	void loadPositions();
	std::ifstream loadFile;
	double* loadBuffer;
};