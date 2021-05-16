#include "application.h"

//Array to store the color values of each pixel on the screen.
//It can't be declared in the class as this would be stored on the stack and the array is too large for that.
//It needs to be declared in the cpp file as the header file is included more than once.
//I could have made this a vector so elements would be stored on the heap.
uint32_t image_data_2D[HEIGHT][WIDTH];

Application::Application(sf::RenderWindow* window, Input* input) :
	window_(window),
	input_(input)
{
	file = new std::ofstream("temp.csv");

	//sf::Uint8 vector needs to be 4 times longer than the uint32_t array.
	image_data_1D = new std::vector<sf::Uint8>();
	image_data_1D->assign(HEIGHT * WIDTH * 4, 0);

	//Creating instances of objects for rendering.
	image_ = new sf::Image();
	texture_ = new sf::Texture();
	sprite_ = new sf::Sprite();

	//Population my accelerators vector.
	std::vector<concurrency::accelerator> accs = concurrency::accelerator::get_all();
	for (auto a : accs)
		if (!a.is_emulated)
			accelerators.push_back(a);

//If I only want to use one GPU and there is two, delete the last one.
#if SINGLE_GPU
	if(accelerators.size() > 1)
	accelerators.pop_back();
#endif

	farm = new Farm(accelerators);

	//Printing the properties of the accelerators on the system.
	query_AMP_support();

	//Creating an empty sprite as the first time this is done it takes longer so I didn't want it messing with my benchmarking.
	image_->create(WIDTH, HEIGHT, image_data_1D->data());
	texture_->loadFromImage(*image_);
	sprite_->setTexture(*texture_);

	//Offsetting the sprite if in fullscreen mode.
#if VITA_LAB
	sprite_->setPosition(0, 28);
#endif

	//Setting the speed depending on how far zoomed in I am.
	speed_ = Map(1 + zoom_, 1, 0, 0.1, 0);

	//Benchmarking
	//for (int j = 0; j < 100; j++)
	//{
	//	std::cout << j << std::endl;
	//	for (int i = 1; i <= 32; i++)
	//	{
	//		//26 : 6 is the optimum load share for the computers in room 3516.
	//		concurrency::extent<2> ext = concurrency::extent<2>(8 * (128 - i), WIDTH);
	//		farm->AddTask(new MandelbrotTask(ext, 0, image_data_2D[0], x_, y_, zoom_));
	//		ext = concurrency::extent<2>(8 * i, WIDTH);
	//		farm->AddTask(new MandelbrotTask(ext, 8 * (128 - i), image_data_2D[8 * (128 - i)], x_, y_, zoom_));
	//		std::chrono::steady_clock::time_point start;
	//		std::chrono::steady_clock::time_point end;
	//		start = std::chrono::steady_clock::now();
	//		farm->Run();
	//		end = std::chrono::steady_clock::now();
	//		*file << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << ",";
	//	}
	//	*file << std::endl;
	//}
}

Application::~Application()
{
	//Cleaning up.
	delete image_data_1D;
	delete image_;
	delete texture_;
	delete sprite_;
}

void Application::HandleInput()
{
	//Exit the application.
	if (input_->isKeyDown(sf::Keyboard::Escape))
	{
		running_ = false;
	}

	//Different delta mouse coordinates for fullscreen as not having to take border into account.
#if VITA_LAB
	delta_mouse_x_ = (input_->getMouseX()) - 960;
	delta_mouse_y_ = (input_->getMouseY()) - 540;
#else
	delta_mouse_x_ = (input_->getMouseX()) - 312;
	delta_mouse_y_ = (input_->getMouseY()) - 209;
#endif

	//Tring to save the GPU some work if I am not using the mouse but it is buggy.
	//if (!(delta_mouse_x_ == 0 && delta_mouse_y_ == 0))
	{
		//Panning around the image.
		x_ += delta_mouse_x_ * speed_ *0.01;
		y_ -= delta_mouse_y_ * speed_ *0.01;

		//std::cout << x_ << "\n" << y_ << "\n" << zoom_ << "\n\n";

		//Zooming in and out and adjusting speed.
		if (input_->isMouseLeftDown())
		{
			if (zoom_ > max_zoom_)
			{
				speed_ = Map(1 + zoom_, 1, 0, 0.1, 0);
				zoom_ -= speed_;
			}
		}
		if (input_->isMouseRightDown())
		{
			if (zoom_ < min_zoom_)
			{
				speed_ = Map(1 + zoom_, 1, 0, 0.1, 0);
				zoom_ += speed_;
			}
		}

		//Updating at the end of each frame.
		Update();
	}
}

void Application::Update()
{
	//Time variables for benchmarking.
	//std::chrono::steady_clock::time_point start;
	//std::chrono::steady_clock::time_point end;

	//If there is more than one GPU
	if (accelerators.size() > 1)
	{
//Stupid farm is just allocating a hardcoded amount of work to each GPU.
//This was to try and see an improvement from just using the 1080 graphics card as opposed to the graphics card and integrated graphics card.
//This is only for 2 GPUs.
#if STUPID_FARM
		int x = 144;
		//9 : 64 (144 rows for integrated GPU) is the optimum load share for the computers in room 3516.
		concurrency::extent<2> ext = concurrency::extent<2>(HEIGHT - x, WIDTH);
		farm->AddTask(new MandelbrotTask(ext, 0, image_data_2D[0], x_, y_, zoom_));
		ext = concurrency::extent<2>(x, WIDTH);
		farm->AddTask(new MandelbrotTask(ext, HEIGHT - x, image_data_2D[HEIGHT - x], x_, y_, zoom_));
//This is the proper farming method. I was not able to get an increase in speed by delogating tasks between 
//the 1080 graphics card and the integrated one as the performace differences were too great. 
#else
		//slice_size will need to be played with to get optimum performance on different PCs.
		//Bare in mind if using tiling, the tiles will have to fit the slice size you give it.
		
		concurrency::extent<2> ext = concurrency::extent<2>(TASK_SIZE, WIDTH);
		for (int i = 0; i < HEIGHT; i += TASK_SIZE)
			farm->AddTask(new MandelbrotTask(ext, i, image_data_2D[i], x_, y_, zoom_));
#endif
		//start = std::chrono::steady_clock::now();
		farm->Run();
		//end = std::chrono::steady_clock::now();
		//There is a bottle neck here with proper farming technique where neither GPUs are doing anything. 
		//Can I have the adding tasks running on a seperate thread to the rest of the method?
	}
	//If there is only one GPU
	else 
	{
		//No need to use the farming method as all the tasks will be worked on by one GPU.
		//The extent will be the entire size of the image.
		concurrency::extent<2> ext = concurrency::extent<2>(HEIGHT, WIDTH);
		MandelbrotTask task(ext, 0, image_data_2D[0], x_, y_, zoom_);
		//start = std::chrono::steady_clock::now();
		//Passing it the default accelerator.
		task.Compute(&accelerators[0], 0);
		//end = std::chrono::steady_clock::now();
	}
	//Printing the time it takes for the actual mandelbrot calculation. This is the data I am interested in.
	//I am also timing the convertion and rendering operations to see if frame drops could be caused there as well.
	//*file << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "," << std::endl;
	//std::cout << "compute:\t" << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
	//start = std::chrono::steady_clock::now();
	//Converting the uint32_t data to data sf::Image can use.
	Convert();
	//end = std::chrono::steady_clock::now();
	//std::cout << "convert:\t" << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
	//start = std::chrono::steady_clock::now();
	//Rendering the image.
	image_->create(WIDTH, HEIGHT, image_data_1D->data());
	//end = std::chrono::steady_clock::now();
	//std::cout << "image:\t\t" << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
	//start = std::chrono::steady_clock::now();
	texture_->loadFromImage(*image_);
	//end = std::chrono::steady_clock::now();
	//std::cout << "texture:\t\t" << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
	//start = std::chrono::steady_clock::now();
	sprite_->setTexture(*texture_);
	//end = std::chrono::steady_clock::now();
	//std::cout << "sprite:\t\t" << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl;
	//start = std::chrono::steady_clock::now();
	Render();
	//end = std::chrono::steady_clock::now();
	//std::cout << "render:\t\t" << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << std::endl << std::endl;
}

void Application::Convert()
{
#if 1
	//Copying the data directly from the 2d uint32_t array into the sf::Uint8 vector.
	//As the data will always be the same size I can safely do this and it will interperate memory in the desired way.
	//This is far quicker than my first method.
	memcpy(image_data_1D->data(), image_data_2D, WIDTH * HEIGHT * 4);
#else
	//This was my first method of conversion. It requires a double for loop and an iterator which is far slower.
	//I am keeping it just in case.
	int index = 0;
	for (int i = 0; i < HEIGHT; i++)
		for (int j = 0; j < WIDTH; j++)
		{
			sf::Uint8 r = image_data_2D[i][j];
			sf::Uint8 g = image_data_2D[i][j] >> 8;
			sf::Uint8 b = image_data_2D[i][j] >> 16;
			sf::Uint8 a = 0xFF;
			image_data_1D->at(index++) = r;
			image_data_1D->at(index++) = g;
			image_data_1D->at(index++) = b;
			image_data_1D->at(index++) = a;
		}
#endif
}

void Application::Render()
{
	window_->clear(sf::Color::Black);

	window_->draw(*sprite_);

	window_->display();
}

double Application::Map(long double value, long double min1, long double max1, long double min2, long double max2)
{
	return min2 + (max2 - min2) * ((value - min1) / (max1 - min1));
}

//Print information about the accelerator.
void Application::report_accelerator(const concurrency::accelerator a)
{
	const std::wstring bs[2] = { L"false", L"true" };
	std::wcout << ": " << a.description << " "
		<< std::endl << "       device_path                       = " << a.device_path
		<< std::endl << "       dedicated_memory                  = " << std::setprecision(4) << float(a.dedicated_memory) / (1024.0f * 1024.0f) << " Mb"
		<< std::endl << "       has_display                       = " << bs[a.has_display]
		<< std::endl << "       is_debug                          = " << bs[a.is_debug]
		<< std::endl << "       is_emulated                       = " << bs[a.is_emulated]
		<< std::endl << "       supports_double_precision         = " << bs[a.supports_double_precision]
		<< std::endl << "       supports_limited_double_precision = " << bs[a.supports_limited_double_precision]
		<< std::endl;
}

//List and select the accelerator to use
void Application::list_accelerators()
{
	//Get all accelerators available to us and store in a vector so we can extract details
	std::vector<concurrency::accelerator> accls = concurrency::accelerator::get_all();

	//Iterates over all accelerators and print characteristics
	for (unsigned i = 0; i < accls.size(); i++)
	{
		concurrency::accelerator a = accls[i];
		report_accelerator(a);
	}
	concurrency::accelerator::set_default(accls[0].device_path);
	concurrency::accelerator acc = concurrency::accelerator(concurrency::accelerator::default_accelerator);
	std::wcout << " default acc = " << acc.description << std::endl;
}

//Query if AMP accelerator exists on hardware
void Application::query_AMP_support()
{
	std::vector<concurrency::accelerator> accls = concurrency::accelerator::get_all();
	if (accls.empty())
	{
		std::cout << "No accelerators found that are compatible with C++ AMP" << std::endl;
	}
	else
	{
		std::cout << "Accelerators found that are compatible with C++ AMP" << std::endl;
		list_accelerators();
	}
}
