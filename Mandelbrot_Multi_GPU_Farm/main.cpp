#pragma once
#include "application.h"
#include "input.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
//Setting up the window.
#if VITA_LAB
	sf::RenderWindow window_(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot", sf::Style::Fullscreen);
	//window_.setPosition(sf::Vector2i(1920, 0));
#else
	sf::RenderWindow window_(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot");
	window_.setPosition(sf::Vector2i(600, 90));
#endif
	window_.setMouseCursorVisible(false);
	window_.setFramerateLimit(60);

	sf::Mouse mouse;
	Input input;
	Application application(&window_, &input);

	//Resetting the mouse position to the center of the screen.
#if VITA_LAB
	mouse.setPosition(sf::Vector2i(window_.getPosition().x + 960, window_.getPosition().y + 540));
#else
	mouse.setPosition(sf::Vector2i(window_.getPosition().x + 320, window_.getPosition().y + 240));
#endif
	sf::sleep(sf::milliseconds(100));	

//Looping through events list and writing to Input class when mouse and keyboard events happen.
	while (window_.isOpen())
	{

		if (window_.hasFocus())
		{
			sf::Event event;
			while (window_.pollEvent(event))
			{
				switch (event.type)
				{
				case sf::Event::Closed:
					window_.close();
					break;
				case sf::Event::KeyPressed:
					input.setKeyDown(event.key.code);
					break;
				case sf::Event::KeyReleased:
					input.setKeyUp(event.key.code);
					break;
				case sf::Event::MouseMoved:
				{
					int asdf = event.mouseMove.x;
					input.setMousePosition(event.mouseMove.x, event.mouseMove.y);
					break;
				}
				case sf::Event::MouseButtonPressed:
					if (event.mouseButton.button == sf::Mouse::Left)
						input.setMouseLeftDown(true);
					if (event.mouseButton.button == sf::Mouse::Right)
						input.setMouseRightDown(true);
					break;
				case sf::Event::MouseButtonReleased:
					if (event.mouseButton.button == sf::Mouse::Left)
						input.setMouseLeftDown(false);
					if (event.mouseButton.button == sf::Mouse::Right)
						input.setMouseRightDown(false);
					break;
				default:
					break;
				}
			}
			application.HandleInput();

			//Resetting the mouse position to the center of the screen.
#if VITA_LAB
			mouse.setPosition(sf::Vector2i(window_.getPosition().x + 960, window_.getPosition().y + 540));
#else
			mouse.setPosition(sf::Vector2i(window_.getPosition().x + 320, window_.getPosition().y + 240));
#endif

//If 'escape' is pressed, the application is not running and closes.
			if (!application.GetRunning())
				window_.close();


			
		}
		//if (c++ == 1000)
			//window_.close();
		
	}

	return 0;
}

