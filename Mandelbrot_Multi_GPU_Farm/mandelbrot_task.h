#pragma once

#include "task.h"

//Pre processing boolean that states if I am working ing the vita lab or not.
#define VITA_LAB 1

//Pre processing boolean that states if I want to use just the main GPU or not.
#define SINGLE_GPU 0

//Creating a typedef that is either a float or double depending on whether the GPU I am working with supports doubles.
#if VITA_LAB
using decimal = double;
#else
using decimal = float;
#endif

//I use a bigger extent in the vita lab as they are more powerfull computers and I want want to test their limits.
#if VITA_LAB
const int WIDTH = 1920;
const int HEIGHT = 1024;
#else
const int WIDTH = 640;
const int HEIGHT = 480;
#endif

//The higher the iterations, the more detailed the image.
const int MAX_ITERATIONS = 500;

//The size of the tasks in rows.
const int TASK_SIZE = 64;

//Tile size.
const int TS_X = 8;
const int TS_Y = 8;

//Complex number struct.
struct Complex1
{
	double x;
	double y;
};

class MandelbrotTask : public Task
{
public:
	MandelbrotTask(concurrency::extent<2> ext, int start_row, uint32_t* image, decimal x, decimal y, decimal zoom);
	~MandelbrotTask();

	//Pointer to where the arrayview will be looking at.
	uint32_t* image_;

	//Constant variables to keep aspect ratio of the image.
	const decimal left_ = -1.5;
	const decimal right_ = 1.5;
	const decimal top_ = 1.125;
	const decimal bottom_ = -1.125;

	//Center point of the current view.
	decimal x_;
	decimal y_;
	//How far zoomed in it is.
	decimal zoom_;

	//The extent of the task.
	concurrency::extent<2> ext_;

	decimal start_row_;

	//Compute Mandelbrot.
	void Compute(concurrency::accelerator* acc, int id);

};

