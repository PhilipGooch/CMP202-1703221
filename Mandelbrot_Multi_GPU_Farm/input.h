#pragma once
#include <SFML/Graphics.hpp>

//A calss to hold and manipulate information about keyboard and mouse inputs
class Input
{
public:
	Input();
	~Input();
	void setKeyDown(int);
	void setKeyUp(int);
	bool isKeyDown(int);
	void setMouseX(int);
	void setMouseY(int);
	void setMousePosition(int, int);
	void setLastMousePosition(int, int);
	int getMouseX();
	int getMouseY();
	void setMouseLeftDown(bool);
	bool isMouseLeftDown();
	void setMouseRightDown(bool);
	bool isMouseRightDown();
	int getLastMouseX();
	int getLastMouseY();

private:
	struct Mouse
	{
		int lastX, lastY;
		int x, y;
		bool left = false;
		bool right = false;
		bool dragging = false;
	};
	bool keys[256]{ false };
	Mouse mouse;
};