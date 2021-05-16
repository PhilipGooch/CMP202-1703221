#include "farm.h"

// FIXME - You may need to add #includes here (e.g. <thread>)

Farm::Farm(std::vector<concurrency::accelerator>& accelerators) :
	accelerators_(accelerators)
{
}

Farm::~Farm()
{
}

void Farm::AddTask(Task *task)
{
	//Only allocating one task per thread of set sizes.
	//Unlocks mutex when it goes out of scope.
	std::unique_lock<std::mutex> lock(task_mutex);
	tasks.push(task);
}

void Farm::Run()
{
//Each thread will only have one task of different sizes.
#if STUPID_FARM
	for (int i = 0; i < accelerators_.size(); i++)
	{
		//One thread per accelerator
		threads.push_back(new std::thread([=]
		{
			//Only allowing one thread access to the task queue.
			task_mutex.lock();
			//Grabbing a task off the front of the queue.
			Task* task = tasks.front();
			tasks.pop();
			task_mutex.unlock();
			//Running the task.
			task->Compute(&accelerators_[i], i);
			delete task;
		}));
	}
#else
	for (int i = 0; i < accelerators_.size(); i++) 
	{
		//One thread per accelerator
		threads.push_back(new std::thread([=] 
		{
			//Loop until the tasks queue is empty.
			//The while(true) instead of while(!tasks.empty()) is to fit in mutex.
			while (true) 
			{
				task_mutex.lock();
				//If the tasks queue is not empty.
				if (!tasks.empty()) 
				{
					//Only allowing one thread access to the task queue.
					Task* task = tasks.front();
					//Grabbing a task off the front of the queue.
					tasks.pop();
					task_mutex.unlock();
					//Running the task.
					task->Compute(&accelerators_[i], i);
					delete task;
				}
				//If the tasks queue is empty.
				else 
				{
					task_mutex.unlock();
					return;
				}
			}
		}));
	}
#endif
	//Waiting for all the threads to complete.
	for (auto th : threads) 
	{
		th->join();
	}
	//Clearing the queue.
	threads.clear();
}
