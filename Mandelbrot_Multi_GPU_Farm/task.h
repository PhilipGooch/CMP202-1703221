#pragma once
#include <amp.h>

//Base task class for if i wanted to have different tasks.
class Task
{
public:
	virtual ~Task() {}

	//Runs the task.
	virtual void Compute(concurrency::accelerator* acc, int id) = 0;
};
