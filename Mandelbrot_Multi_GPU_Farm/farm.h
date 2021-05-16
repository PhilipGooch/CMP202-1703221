#pragma once

#include "mandelbrot_task.h"

#include <queue>
#include <mutex>
#include <thread>
#include <vector>

//Stupid farm states whether it is using the scatter - gather technique or just splitting it in two with hard coded values.
#define STUPID_FARM 0

class Farm {
public:
	Farm(std::vector<concurrency::accelerator>& accelerators);
	~Farm();

	//Method to add a class to the task queue.
	void AddTask(Task *task);

	//Method to delogate and run tasks on multiple GPUs.
	void Run();

private:
	//Queue of tasks to be shared between GPUs.
	std::queue<Task*> tasks;

	//Vector of thread pointers.
	std::vector<std::thread*> threads;

	//Mutex to block threads from accelssing the shared resource at the same time.
	std::mutex task_mutex;

	//Reference to the vector of accelerators in application.h
	std::vector<concurrency::accelerator>& accelerators_;		

};
