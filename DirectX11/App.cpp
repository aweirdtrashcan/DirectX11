#include "App.h"

App::App()
	:
	wnd(800, 600, "The Donkey Fart Box")
{}

int App::Go()
{
	while (true)
	{
		// process all messages pending, but to not block for new messages
		if (const auto ecode = Window::ProcessMessages())
		{
			// if return optional has value, means we're quitting so return exit code
			return *ecode;
		}
		DoFrame();
	}
}

void App::DoFrame()
{
	const auto c = sin(timer.Peek()) / 2.0f + 0.5f;
	wnd.Gfx().ClearBuffer(c, c, 1.0f);
	//wnd.Gfx().DrawTestTriangle(
	//	timer.Peek(),
	//	(wnd.mouse.GetPosX() / 400.f) - 1.0f,
	//	(-wnd.mouse.GetPosY() / 300.f) + 1.0f
	//);

	if (wnd.kbd.KeyIsPressed('W'))
	{
		yDelta += 0.05f;
	}

	if (wnd.kbd.KeyIsPressed('S'))
	{
		yDelta -= 0.05f;
	}

	if (yDelta > 1.5f)
	{
		yDelta = 1.5f;
	}

	if (yDelta < -1.5f)
	{
		yDelta = -1.5f;
	}

	if (wnd.kbd.KeyIsPressed('D'))
	{
		xDelta += 0.05f;
	}

	if (wnd.kbd.KeyIsPressed('A'))
	{
		xDelta -= 0.05f;
	}

	if (xDelta > 2.50f)
	{
		xDelta = 2.50f;
	}

	if (xDelta < -2.50f)
	{
		xDelta = -2.5f;
	}

	wnd.Gfx().DrawTestTriangle(
		timer.Peek(),
		xDelta, 0.8, yDelta
	);

	wnd.Gfx().EndFrame();
}