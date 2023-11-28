#include <SFML/Graphics.hpp>

#include "Particles.hpp"
#include "Particles2DArray.hpp"
#include "PPC.hpp"
#include "NoInteraction.hpp"


int main()
{
	//srand((unsigned int)time(NULL));

	// ---- Creating window ----
	uint16_t sizeX = 1900;
	uint16_t sizeY = 1000;
	sf::RenderWindow window(sf::VideoMode(sizeX, sizeY), "LGPPE sim", sf::Style::Default);
	sf::RenderTarget& renderTarget= window;
	//window.setFramerateLimit(80);

	window.setKeyRepeatEnabled(false);
	sf::View view = window.getDefaultView(); // https://www.sfml-dev.org/tutorials/2.0/graphics-view-fr.php
											// https://www.sfml-dev.org/documentation/2.5.1/classsf_1_1View.php
	view.zoom(1);
	window.setView(view);

	// ---- view and interactions ----
	bool rightMousePressed = 0;
	sf::Vector2u initialMousePos;
	sf::Vector2f initialCenterPos;
	sf::Vector2u windowSize = window.getSize();
	sf::Vector2f viewSize = view.getSize();

	// ---- Creating simulator ----
	uint16_t tileSize = 10;
	//Particles pp;
	//Particles2DArray pp(sizeX/ tileSize, sizeY/ tileSize, tileSize);
	//PPC pp(sizeX/ tileSize, sizeY/ tileSize, tileSize);
	NoInteraction pp(sizeX, sizeY);


	// ---- FPS tracking ----
	uint32_t nbFrames = 1;
	clock_t sum = 0;
	clock_t start = clock(), end;

	sf::Font font;
	if (!font.loadFromFile("OpenSans-Regular.ttf")) std::cout << "error loading arial.ttf" << std::endl;
	
	sf::Text FPS_displayer;
	FPS_displayer.setFont(font);
	FPS_displayer.setString("0");
	FPS_displayer.setCharacterSize(24);
	FPS_displayer.setFillColor(sf::Color::White);
	FPS_displayer.setPosition(0.f, 0.f);

	// ---- Simulations saving ----
	uint8_t saving = 0; // 0 do nothing, 1 save, 2 load
	uint64_t maxSimulationDuration = 30000;
	if (saving == 1) {
		pp.openFileForSaving();
	}
	else if (saving == 2) {
		pp.openFileForLoading();
	}


	// ---- Main loop ----
	while (window.isOpen() && (bool)(saving) * pp.world.time < maxSimulationDuration)
	{
		sf::Event event; // https://www.sfml-dev.org/tutorials/2.6/window-events.php
		while (window.pollEvent(event))
		{
			switch (event.type) {
			case sf::Event::KeyPressed :
				if (event.key.scancode == sf::Keyboard::Scan::Escape) {
					window.close();
				}
				break;
			case sf::Event::MouseWheelScrolled :
				view.zoom((float)(6+ event.mouseWheelScroll.delta)/6);
				window.setView(view);
				break;

			case sf::Event::MouseButtonPressed :
				if (event.mouseButton.button == sf::Mouse::Right) {
					rightMousePressed = 1;
					initialMousePos.x = event.mouseButton.x;
					initialMousePos.y = event.mouseButton.y;
					initialCenterPos = view.getCenter();
				}
				break;

			case sf::Event::MouseButtonReleased:
				if (event.mouseButton.button == sf::Mouse::Right) {
					rightMousePressed = 0;
				}
				break;

			case sf::Event::MouseMoved :
				if (rightMousePressed) {
					windowSize = window.getSize();
					viewSize = view.getSize();

					view.setCenter(initialCenterPos.x + ((float)initialMousePos.x - event.mouseMove.x) * viewSize.x / windowSize.x, 
								   initialCenterPos.y + ((float)initialMousePos.y - event.mouseMove.y) * viewSize.y / windowSize.y);

					window.setView(view);
				}
				break;
			case sf::Event::Closed :
				window.close();
				break;
			}
		}

		// Physics
		if (saving == 2) {
			pp.world.time++;
			pp.loadPositions(); // doesn't load speed so watch out if UpVertices uses particles' speed
		}
		else {
			pp.solver();
			if (saving == 1) {
				pp.savePositions();
			}
		}

		pp.UpVertices();
		
		window.clear(sf::Color(0, 0, 10, 100));
		//window.setView(view);
		window.draw(pp);
		//window.setView(window.getDefaultView());
		window.draw(FPS_displayer);
		window.display();


		end = clock();
		sum += end - start;
		start = clock();
		nbFrames++;
		if (sum > CLOCKS_PER_SEC) {
			char fpsstr[10]; // enough to hold all numbers up to 32-bits
			sprintf_s(fpsstr, "%I32i", nbFrames);
			//char timestr[21]; // enough to hold all numbers up to 64-bits
			//sprintf_s(timestr, "%I64i", pp.world.time); // To write world time (in ticks) to displayer
			FPS_displayer.setString(fpsstr);
			sum = 0;
			nbFrames = 0;
		}
	}

	/*if (window.isOpen()) {
		window.close();
	}*/

	// ---- End of Simulations saving ----
	if (saving == 1) {
		pp.closeFileForSaving();
	}
	else if (saving == 2) {
		pp.closeFileForLoading();
	}

	if (pp.world.time == maxSimulationDuration) {
		std::cout << "Simulation finished (time reached)" << std::endl;
	}
	//delete &pp;
	return 0;
}