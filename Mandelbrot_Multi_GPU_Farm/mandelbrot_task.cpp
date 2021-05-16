#include "mandelbrot_task.h"

//Function to add two complex numbers. 
//Restrict keyword - able to execute this function on the GPU and CPU.
Complex1 c_add(Complex1 c1, Complex1 c2) restrict(cpu, amp) 
{
	Complex1 tmp;
	decimal a = c1.x;
	decimal b = c1.y;
	decimal c = c2.x;
	decimal d = c2.y;
	tmp.x = a + c;
	tmp.y = b + d;
	return tmp;
} 

//Function to multiply two complex numbers. 
Complex1 c_mul(Complex1 c1, Complex1 c2) restrict(cpu, amp)
{
	Complex1 tmp;
	decimal a = c1.x;
	decimal b = c1.y;
	decimal c = c2.x;
	decimal d = c2.y;
	tmp.x = a * c - b * d;
	tmp.y = b * c + a * d;
	return tmp;
} // c_mul

MandelbrotTask::MandelbrotTask(concurrency::extent<2> ext, int start_row, uint32_t* image, decimal x, decimal y, decimal zoom) :
	ext_(ext),
	start_row_(start_row),
	image_(image),
	x_(x),
	y_(y),
	zoom_(zoom)
{
}

MandelbrotTask::~MandelbrotTask()
{
}

//Computing the value of each pixel in the extent of the mandelbrot given.
void MandelbrotTask::Compute(concurrency::accelerator* acc, int id)
{
	//Creating an instance of an accelerator view using the default view of the accelerator that is passed in.
	concurrency::accelerator_view view = acc->default_view;

	//Creating an array_view with the dimensions and memory location it is looking at.
	concurrency::array_view<uint32_t, 2> image(ext_, image_);

	//Creating temporary variables used to keep aspect ratio the image.
	//Calculated using the mouse position and zoom.
	//The top and bottom calculations are more complicated as they change depending on the task ID. It works for any task size.
	decimal l = x_ + left_ + (left_ * zoom_);
	decimal r = x_ + right_ + (right_ * zoom_);
	decimal t = y_ - ((top_ / (HEIGHT / 2)) * start_row_) * (1 + zoom_) + top_ + (top_ * zoom_);		
	decimal b = y_ - ((top_ / (HEIGHT / 2)) * start_row_) * (1 + zoom_) + bottom_ + (bottom_ * zoom_);

//Swith betweed tiled and not tiled index.
#define TILED  1

#if TILED 
	//Passing the accelerator view it is working with, the dimensions in the form on an extent and the lambda with the mandelbrot code in.
	concurrency::parallel_for_each(view, image.extent.tile<TS_Y, TS_X>(), [=](concurrency::tiled_index<TS_Y, TS_X> t_idx) restrict(amp)
	{
#else
	concurrency::parallel_for_each(view, image.extent, [=](concurrency::index<2> idx) restrict(amp)
	{
#endif
#if TILED
		int x = t_idx.global[1];
		int y = t_idx.global[0];
#else
		int x = idx[1];
		int y = idx[0];
#endif

		// Work out the point in the complex plane that
		// corresponds to this pixel in the output image.
		Complex1 c;
		c.x = l + (x * (r - l) / WIDTH);
		c.y = t + (y * (b - t) / HEIGHT);

		// Start off z at (0, 0).
		Complex1 z;
		z.x = 0;
		z.y = 0;

		// Iterate z = z^2 + c until z moves more than 2 units
		// away from (0, 0), or we've iterated too many times.
		int iterations = 0;

		while ((z.x * z.x) + (z.y * z.y) < 2.0 * 2.0 && iterations < MAX_ITERATIONS)
		{
			z = c_add(c_mul(z, z), c);
			++iterations;
		}
		if (iterations == MAX_ITERATIONS)
		{
			// z didn't escape from the circle.
			// This point is in the Mandelbrot set.
			if (id == 0)
				image(y, x) = 0xFFFF0000;
			else
				image(y, x) = 0xFF0000FF;
		}
		else
		{
			// z escaped within less than MAX_ITERATIONS
			// iterations. This point isn't in the set.
			if(id == 0)
				image(y, x) = 0xFF000000 + ((iterations % 255) << 16);
			else
				image(y, x) = 0xFF000000 + (iterations);
			//av_image(y, x) = colourHSVtoABGR(iterations, 0.8, 0.8);
		}
	});
	//Synchronising the arrayview on the GPU with the array on the CPU.
	image.synchronize();
}
