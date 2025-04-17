/*#include "windows\glew\include\GL\glew.h"
#include "windows\glut\glut.h"
#include "CollisionBox.h"
#include "EnvironmentEntity.h"
#include "GameEntity.h"
#include "Globals.h"
#include "MobileEntity.h"
#include "Player.h"
#include "WorldEntity.h"
*/
//#include "GL\glew.h"
//#include "GL\glut.h"
#include "SoundPlayer.h"
#include "General.h"
#include "Gamepad.h"
#include "SDL.h"
#include <cstring>
#ifdef __EMSCRIPTEN__
#	include <emscripten.h>
#endif
#ifdef _WIN32
#	include <direct.h> // for chdir (deprecated, replace with _chdir)
#else
#	include <unistd.h> // for chdir
#endif

using namespace Globals;

static constexpr int FRAME_INTERVAL_MS = 1000 / 30;
static constexpr int WINDOW_WIDTH  = 600;
static constexpr int WINDOW_HEIGHT = 600;

static void eventLoop();
static void updateAndDraw();

static Uint32 mainTimerEventType;
static Uint32 onMainTimer(Uint32 interval, void*);

void initSDL() {
	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS|SDL_INIT_GAMECONTROLLER);
#ifdef __EMSCRIPTEN__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	mainWindow = SDL_CreateWindow("G Test", 100, 50, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
	SDL_GLContext glcontext = SDL_GL_CreateContext(mainWindow);
	SDL_GL_MakeCurrent(mainWindow, glcontext);
}

int main(int argc, const char** argv){
	initSDL();

#ifdef CMAKE_INSTALL_FULL_DATADIR
	chdir(CMAKE_INSTALL_FULL_DATADIR);
#else
	{
		char* basePath = SDL_GetBasePath();
		if (basePath) {
			chdir(basePath);
			SDL_free(basePath);
		}
	}
#endif

	//initialize glew
#ifndef __EMSCRIPTEN__
	glewExperimental = GL_TRUE;
	const GLenum glewStatus = glewInit();
	if (glewStatus != GLEW_OK) {
		std::cerr << "Error initializing GLEW library: " << glewGetErrorString(glewStatus) << '\n';
		return 1;
	}
#endif
	initApp();

	//initialize game controller
	Gamepad::initializeGamepad();

	SoundPlayerGuard spg;
	if(spg.initialized) {
#ifdef __EMSCRIPTEN__
			if(!SoundPlayer::playBackground("resources/cl1.mp3"))
				std::cerr<<"could not play file cl1.mp3. \n";
#else
			if(!SoundPlayer::playBackground("resources/cl1.midi"))
				std::cerr<<"could not play file cl1.midi. \n";
#endif
			if(!SoundPlayer::loadSound("resources/curvy.wav"))
				std::cerr<<"could not play file curvy.way. \n";
			if(!SoundPlayer::loadSound("resources/cannon.wav"))
				std::cerr<<"could not play file cannon.wav. \n";
			if(!SoundPlayer::loadSound("resources/death.wav"))
				std::cerr<<"could not play file death.wav. \n";
			if(!SoundPlayer::loadSound("resources/fireball.wav"))
				std::cerr<<"could not play file fireball.wav . \n";
			if(!SoundPlayer::loadSound("resources/jump.wav"))
				std::cerr<<"could not play file jump.wav. \n";
	} else {
		std::cerr<<"SoundPlayer failed to initialize.\n";
	}

	mainTimerEventType = SDL_RegisterEvents(1);
	if (mainTimerEventType == -1) {
		std::cerr << "Fatal error: failed to register main timer event.\n";
		return 1;
	}
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(eventLoop, 0, 1);
#else
	SDL_AddTimer(FRAME_INTERVAL_MS, onMainTimer, NULL);

	eventLoop();
	SDL_Quit();
#endif

	return 0;
}

#ifdef __EMSCRIPTEN__
extern "C" {
	void invokeMain() {
		const char* argv[] = { "cs174a_term_project" };
		main(1, argv);
	}
	// This is called when the program first starts if the intro screen
	// was already put into fullscreen.
	void setFullscreen() {
		std::cerr << "Attempting to set fullscreen\n";
		Globals::viewFullscreen();
	}
	void setPause(bool pause) {
		Globals::setPause(pause);
	}
}
#endif

Uint32 onMainTimer(Uint32 interval, void*) {
	// This callback is called on a separate thread. We have to push an
	// event to be executed on the main thread to avoid thread problems.
	SDL_Event event;
	event.user.type = mainTimerEventType;
	// event.user.timestamp is set by SDL_PushEvent
	event.user.windowID = 0;
	event.user.code = 0;
	event.user.data1 = NULL;
	event.user.data2 = NULL;
	SDL_PushEvent(&event);
	return interval;
}

void eventLoop() {
#ifdef __EMSCRIPTEN__
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
#else
	while(true) {
		SDL_Event event;
		if (!SDL_WaitEvent(&event)) {
			std::cerr << "error encountered: " << SDL_GetError() << '\n';
			return;
		}
#endif

		switch(event.type) {
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
#ifdef __EMSCRIPTEN__
				// HACK: The browser expects the canvas to always render at a constant size
				//       when not in fullscreen mode.
				if (!(SDL_GetWindowFlags(mainWindow) & SDL_WINDOW_FULLSCREEN)) {
					callbackReshape(WINDOW_WIDTH, WINDOW_HEIGHT);
					break;
				}
#endif				
				callbackReshape(event.window.data1, event.window.data2);
			}
			break;
		case SDL_KEYDOWN: {
			int mouseX, mouseY;
			SDL_GetMouseState(&mouseX, &mouseY);
			if(event.key.keysym.sym <= 127)
				callbackKeyboard(event.key.keysym.sym, mouseX, mouseY);
			else
				callbackSpecialFunc(event.key.keysym.sym, mouseX, mouseY);
			break;
		}
		case SDL_KEYUP: {
			int mouseX, mouseY;
			SDL_GetMouseState(&mouseX, &mouseY);
			if(event.key.keysym.sym <= 127)
				callbackKeyboardUp(event.key.keysym.sym, mouseX, mouseY);
			else
				callbackSpecialUpFunc(event.key.keysym.sym, mouseX, mouseY);
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			callbackMouse(event.button.button, event.button.state, event.button.x, event.button.y);
			break;
		case SDL_MOUSEMOTION:
#ifdef __EMSCRIPTEN__
			// Suppress mouse motion events from the first two frames
			// In the browser, there might be an even generated before the mouse is centered,
			// which would cause the game to start with the tank facing some other direction.
			if (Globals::frameCount <= 1) { break; }
#endif
			if (event.motion.state)
				callbackMotion(event.motion.xrel, event.motion.yrel);
			else
				callbackPassiveMotion(event.motion.xrel, event.motion.yrel);
			break;
		case SDL_QUIT:
			Gamepad::shutdownGamepad();
			return;
		default:
			if (Gamepad::processEvent(event))
				break;
			// We check user events here. Because their type IDs are determined at runtime,
			// we can't check them directly with the switch construct.
			if (event.type == mainTimerEventType) {
				updateAndDraw();
			}
			break;
		}
	}
#ifdef __EMSCRIPTEN__
	static Uint64 lastFrameTick = 0;
	const Uint64 currentTick = SDL_GetTicks64();
	if (currentTick - lastFrameTick >= FRAME_INTERVAL_MS) {
		lastFrameTick = currentTick;
		updateAndDraw();
	}
#endif
}

void updateAndDraw() {
	Gamepad::tick();
	callbackTimer(0);

	// Rumble the gamepad when the player's health has changed
	// (i.e., when they're damaged or killed).
	if (Gamepad::hasGamepad()) {
		auto player = Globals::getPlayer();
		if (player) {
			auto health = player->getHealth();
			static auto prevHealth = health;

			if (health != prevHealth) {
				prevHealth = health;
				Gamepad::rumble();
			}
		}
	}
}
