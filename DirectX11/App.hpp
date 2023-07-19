#pragma once
#include "Window.hpp"
#include "ChiliTimer.hpp"

class App
{
public:
	App();
	// master frame / message loop
	int Go();
private:
	void DoFrame();
private:
	Window wnd;
	ChiliTimer timer;
};