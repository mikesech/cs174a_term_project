#ifndef _GAMEPAD
#define _GAMEPAD

#include "SDL_events.h"

namespace Gamepad {
	void initializeGamepad();
	void initializeGamepad(int deviceID);
	void shutdownGamepad();

	bool hasGamepad();
	const char* getName();

	void rumble();

	bool processEvent(const SDL_Event& event);
	void tick();
}

#endif // _GAMEPAD
