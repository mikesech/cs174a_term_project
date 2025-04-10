#include "Gamepad.h"
#include "General.h"
#include "SDL.h"
#ifdef __EMSCRIPTEN__
#	include <emscripten.h>
#endif

static constexpr int GAMEPAD_TRIGGER_THRESHOLD = SDL_JOYSTICK_AXIS_MAX / 16;
static constexpr int GAMEPAD_STICK_THRESHOLD   = SDL_JOYSTICK_AXIS_MAX / 2.5;
static constexpr int GAMEPAD_STICK_DEADZONE    = SDL_JOYSTICK_AXIS_MAX / 4.095875;
static constexpr int GAMEPAD_LOOK_DIVISOR      = 1024;

static SDL_GameController* controller = nullptr;
static Sint32 controllerID = -1;
static int controllerDeviceID = -1; // Only meaningful on Emscripten platform
static int controllerLookX = 0, controllerLookY = 0;

using Globals::callbackKeyboard;
using Globals::callbackKeyboardUp;
using Globals::callbackMouse;
using Globals::callbackPassiveMotion;

// mouseX and mouseY are required by the relevant callbacks (which derive their
// signatures from GLUT), but they aren't actually used. As a result, we just pass
// in 0 for both.
static constexpr int mouseX = 0, mouseY = 0;

static void convertStickToKeystroke(char negativeKey, char positiveKey, Sint16 value) {
	// This might look excessive. It should really only call one of these functions
	// when the value crosses the threshold. However, callbackKeyboard(Up) is
	// inexpensive; it just sets a global variable somewhere.
	if (value <= -GAMEPAD_STICK_THRESHOLD) {
		callbackKeyboard(negativeKey, mouseX, mouseY);
	}
	if (value > -GAMEPAD_STICK_THRESHOLD) {
		callbackKeyboardUp(negativeKey, mouseX, mouseY);
	}
	if (value >= GAMEPAD_STICK_THRESHOLD) {
		callbackKeyboard(positiveKey, mouseX, mouseY);
	}
	if (value < GAMEPAD_STICK_THRESHOLD) {
		callbackKeyboardUp(positiveKey, mouseX, mouseY);
	}
}
static void convertTriggerToClick(int button, Sint16 value) {
	auto state = (value >= GAMEPAD_TRIGGER_THRESHOLD) ? SDL_PRESSED : SDL_RELEASED;
	callbackMouse(button, state, mouseX, mouseY);
}
static void convertButtonToKeystroke(char key, Uint8 state) {
	if (state == SDL_PRESSED)
		callbackKeyboard(key, mouseX, mouseY);
	else
		callbackKeyboardUp(key, mouseX, mouseY);
}
static void convertButtonToClick(int button, Uint8 state) {
	callbackMouse(button, state, mouseX, mouseY);
}

template <typename T>
static T zeroDeadzone(T value, T deadzone = GAMEPAD_STICK_DEADZONE) {
	if (value >= -deadzone && value <= deadzone)
		return 0;
	else
		return value;
}

namespace Gamepad {
	void initializeGamepad() {
		if (controller)
			return;

		for (int i = 0; i < SDL_NumJoysticks(); i++) {
			if (SDL_IsGameController(i)) {
				controller = SDL_GameControllerOpen(i);
				if (controller) {
					controllerDeviceID = i;
					controllerID = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
					break;
				}
			}
		}
	}
	void initializeGamepad(int deviceID) {
		if (controller)
			return;

		controller = SDL_GameControllerOpen(deviceID);
		if (controller) {
			controllerDeviceID = deviceID;
			controllerID = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller));
		}
	}

	void shutdownGamepad() {
		if (!controller)
			return;

		SDL_GameControllerClose(controller);
		controller = nullptr;
		controllerDeviceID = -1;
		controllerID = -1;
		controllerLookX = controllerLookY = 0;
	}

	bool hasGamepad() {
		return !!controller;
	}
	const char* getName() {
		const char* name = SDL_GameControllerName(controller);
		return name ? name : "none";
	}

	void rumble() {
#ifdef __EMSCRIPTEN__
		EM_ASM({
			// XXX: The caching mechanism here is a bit overkill
			globalThis.damageVibrators ||= [];
			let vibrator = globalThis.damageVibrators[$0];
			if (vibrator === undefined) {
				vibrator = navigator.getGamepads()[$0].vibrationActuator ?? null;
				globalThis.damageVibrators[$0] = vibrator;
			}
			if (vibrator != null) {
				vibrator.playEffect('dual-rumble', {
					startDelay: 0,
					duration: 100,
					weakMagnitude: 0,
					strongMagnitude: 0.125
				}).catch((error) => {
					console.warn('Failed rumbling controller: %s', error);
				});
			}
		}, controllerDeviceID);
#else
		SDL_GameControllerRumble(controller, 0xFFFF>>3, 0, 100);
#endif
	}

	bool processEvent(const SDL_Event& event) {
		switch(event.type) {
		case SDL_CONTROLLERDEVICEADDED:
			// This doesn't do anything if a controller is already setup
			initializeGamepad(event.cdevice.which);
			break;
		case SDL_CONTROLLERDEVICEREMOVED:
			if (controller && event.cdevice.which == controllerID) {
				shutdownGamepad();
				// Switch to the next gamepad
				initializeGamepad();
			}
			break;
		case SDL_CONTROLLERAXISMOTION:
			if (controller && event.cdevice.which == controllerID) {
				const auto value = zeroDeadzone(event.caxis.value);
				switch(event.caxis.axis) {
					case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
						convertTriggerToClick(SDL_BUTTON_LEFT, value);
						break;
					case SDL_CONTROLLER_AXIS_LEFTX:
						convertStickToKeystroke('A', 'D', value);
						break;
					case SDL_CONTROLLER_AXIS_LEFTY:
						convertStickToKeystroke('W', 'S', value);
						break;
					case SDL_CONTROLLER_AXIS_RIGHTX:
						controllerLookX = value / GAMEPAD_LOOK_DIVISOR;
						break;
					case SDL_CONTROLLER_AXIS_RIGHTY:
						controllerLookY = value / GAMEPAD_LOOK_DIVISOR;
						break;
				}
			}
			break;
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			if (controller && event.cdevice.which == controllerID) {
				const auto state = event.cbutton.state;
				switch (event.cbutton.button) {
					case SDL_CONTROLLER_BUTTON_A:
						convertButtonToKeystroke(' ', state);
						break;
					case SDL_CONTROLLER_BUTTON_X:
					case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
						convertButtonToKeystroke('Q', state);
						break;
					case SDL_CONTROLLER_BUTTON_B:
					case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
						convertButtonToKeystroke('E', state);
						break;
					case SDL_CONTROLLER_BUTTON_DPAD_UP:
						convertButtonToKeystroke('W', state);
						break;
					case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
						convertButtonToKeystroke('D', state);
						break;
					case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
						convertButtonToKeystroke('S', state);
						break;
					case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
						convertButtonToKeystroke('A', state);
						break;
					case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
						convertButtonToKeystroke('\'', state);
						break;
					case SDL_CONTROLLER_BUTTON_Y:
						convertButtonToClick(SDL_BUTTON_RIGHT, state);
						break;
				}
			}
			break;
		default:
			return false;
		}
		return true;
	}
	void tick() {
		callbackPassiveMotion(controllerLookX, controllerLookY);
	}
}
