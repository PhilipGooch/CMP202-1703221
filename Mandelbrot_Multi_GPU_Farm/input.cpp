#include "Input.h"
#include <iostream>
using namespace std;

Input::Input()
{
	setMousePosition(0, 0);
	setLastMousePosition(getMouseX(), getMouseY());
}

Input::~Input()
{

}

void Input::setKeyDown(int key)
{
	if (key >= 0)
	{
		keys[key] = true;
	}
}
void Input::setKeyUp(int key)
{
	if (key >= 0)
	{
		keys[key] = false;
	}
}
bool Input::isKeyDown(int key)
{
	return keys[key];
}
void Input::setMouseX(int lx)
{
	mouse.x = lx;
}
void Input::setMouseY(int ly)
{
	mouse.y = ly;
}
void Input::setMousePosition(int lx, int ly)
{
	setMouseX(lx);
	setMouseY(ly);
}
void Input::setLastMousePosition(int lx, int ly)
{
	mouse.lastX = lx;
	mouse.lastY = ly;
}
int Input::getMouseX()
{
	return mouse.x;
}
int Input::getMouseY()
{
	return mouse.y;
}
void Input::setMouseLeftDown(bool down)
{
	mouse.left = down;
}
bool Input::isMouseLeftDown()
{
	return mouse.left;
}
void Input::setMouseRightDown(bool down)
{
	mouse.right = down;
}
bool Input::isMouseRightDown()
{
	return mouse.right;
}

int Input::getLastMouseX()
{
	return mouse.lastX;
}

int Input::getLastMouseY()
{
	return mouse.lastY;
}
