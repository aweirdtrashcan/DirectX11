#include "App.hpp"

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
	const float c = static_cast<float>(sin(timer.Peek()) / 2.0f + 0.5f);
	const float speed = 1.0f / 3000.f;
	if (wnd.kbd.KeyIsPressed('A'))
	{
		wnd.Gfx().xPos += speed;
	}
	if (wnd.kbd.KeyIsPressed('D'))
	{
		wnd.Gfx().xPos -= speed;
	}

	if (wnd.kbd.KeyIsPressed('W'))
	{
		wnd.Gfx().zPos += speed;
	}
	if (wnd.kbd.KeyIsPressed('S'))
	{
		wnd.Gfx().zPos -= speed;
	}

	wnd.Gfx().ClearBuffer(c, c, 1.0f);
	wnd.Gfx().DrawTestTriangle(((float)wnd.mouse.GetPosX() / 400) - 1, -((float)wnd.mouse.GetPosY() / 300) + 1);
	wnd.Gfx().EndFrame();
}