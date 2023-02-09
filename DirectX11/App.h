#pragma once
#include "Window.h"
#include "ChiliTimer.h"

class App
{
public:
	App();
	// master frame / message loop
	int Go();
private:
	void DoFrame();
private:
	ChiliTimer timer;
	Window wnd;
	float yDelta = 0.0f;
	float xDelta = 0.0f;
};