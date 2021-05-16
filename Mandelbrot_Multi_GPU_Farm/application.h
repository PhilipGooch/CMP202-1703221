#pragma once
#include <SFML/Graphics.hpp>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <amp.h>
#include <time.h>
#include <string>
#include <array>
#include <amp_math.h>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <complex>
#include "input.h"
#include "farm.h"
#include <fstream>

class Application
{
public:
	Application(sf::RenderWindow* window, Input* input);
	~Application();

	void HandleInput();
	void Update();
	void Render();

	//Returns if the application is running or not.
	inline bool GetRunning() { return running_; }

private:	
	//Pointers to the instances of sf::RenderWindow and Input declared in main.cpp.
	sf::RenderWindow* window_;
	Input* input_;

	//Class for handeling CPU threads delegating tasks to GPUs.
	Farm* farm;

	std::ofstream* file;

	//Vector used to store the converted RGBA values for the image.
	std::vector<sf::Uint8>* image_data_1D;

	//Setting up objects for image rendering.
	sf::Image* image_;
	sf::Texture* texture_;
	sf::Sprite* sprite_;

	//Is the application running.
	bool running_ = true;

	//The GPUs on the system.
	std::vector<concurrency::accelerator> accelerators;

	//How far the mouse has moved from the center each frame.
	int delta_mouse_x_ = 0;
	int delta_mouse_y_ = 0;

#if 1
	//An interesting position in the Mandelbrot set
	long double x_ = -1.2417935910153268;
	long double y_ = 0.32340979014235266;
	
#else
	//Starting position.
	long double x_ = -0.0;
	long double y_ = 0;
#endif

	//Max zoom is set to a point where it starts to get pixelated.
	long double min_zoom_ = 100000000000;
	long double max_zoom_ = -0.99999999999999623;

	long double zoom_ = 0;
	//The speed in which you can zoom and pan.
	long double speed_;

	//Converts 2D array of uint32_t to a format SFML can use.
	void Convert();

	//Maps a number in a given range to another.
	double Map(long double value, long double min1, long double max1, long double min2, long double max2);

	//Print information about the accelerator.
	void report_accelerator(const concurrency::accelerator a);

	//List and select the accelerator to use
	void list_accelerators();

	//Query if AMP accelerator exists on hardware
	void query_AMP_support();

};

