/*
*	TO DO:
*		- replace round doors with normal doors
*			- make them rectangles too
*			- Very light or very dark (For printing over)
*		- print instructions at the bottom (ttf)
*		- door buttons respond to click
*			- display "picked door"
*				- background lights up blue once clicked
*		- Print "???" over closed, non-picked doors
*		- New buttons "switch" and "don't switch"
*		- Load functions from console-based game
*		- Stats on TOP BAR (total wins / Total YES wins / Total NO wins )
*
*
* IMAGE is simply an image loaded onto a SURFACE!
* We use the IMG_Load function which returns a SURFACE
*
* IMGs loaded onto SURFACES are good for MANIPULATING.
* TEXTURES are good for RENDERING onto other surfaces.
*
* So for tiles I will use TEXTURES.
* And for limbs in the battle screen I will use SURFACES (with images loaded onto them), because I'll sometimes be manipulating them in real time.
* ... or maybe not! SDL_RenderCopyEx (which does flipping) operates on TEXTURES.
* This could be really interesting.
*
*
*
*
* 1. Create WINDOW
* 2. Create RENDERER (which belongs to the window... feed the window into the new renderer in its constructor)
* 3. Create SURFACES (or just get the surface FROM the window).
* 4. BLIT some surfaces onto each other (UNNECESSARY)
* 4. Create TEXTURES (from other surfaces) and the texture belongs to a renderer.
* 5. RENDER the textures onto the WINDOW (screen)
* 6. UPDATE the window with one of the two commands:
*		SDL_UpdateWindowSurface(gWindow);
		SDL_RenderPresent(gRenderer);
		---- > SDL_RenderPresent is MORE EFFICIENT!
* 7. CLEAN UP CODE
*		- It is a MESS
*		- SDL components should be global
*		- Game components can be within the main function, and passed around as references/pointers or as values, in function parameters.
*
*
* IMAGE DISPLAYED SUCCESSFULLY!
* 
* NEXT:
*	- Print instructions
*		- The instructions should be able to CHANGE over time.
*	- Print stats
*	- It should print EVERYTHING each time, but EVERYTHING should be based on state. (don't just print once and leave it until there's a change).
*
*
*/


#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <SDL_ttf.h>
#include <cmath>
#include<vector>
#include<cstdlib>
#include <time.h>

import Door;

using std::string;
using std::cout;
using std::cin;
using std::vector;
using std::to_string;

// global constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 720;
const int PADDING = 20;
const int DOOR_WIDTH = (SCREEN_WIDTH - (PADDING * 4)) / 3;
const int DOOR_HEIGHT = DOOR_WIDTH;
const int DOOR_Y_POSITION = DOOR_HEIGHT;


void exit(SDL_Surface* surface, SDL_Window* window);

int getDoorHorizontalPosition(int doorIndex);
void handleClick(SDL_Event* e);
bool initializeSDL2();

// 3 rectangles to display the door
SDL_Rect doorRects[3];

SDL_Window* mainWindow = NULL;
SDL_Renderer* mainRenderer = NULL;
SDL_Surface* mainWindowSurface = NULL;

// font stuff
TTF_Font* font = NULL;
SDL_Color textColor = { 50, 50, 50 };
SDL_Rect titleTextRect;
SDL_Texture* titleTextTexture = NULL;

// game state stuff

enum class GamePhase {
	uninitialized, chooseDoor, chooseSwitch, gameOver
};

GamePhase gameState = GamePhase::uninitialized;



int main(int argc, char* args[]) {
	cout << "THis is the main function";

	bool initialized = initializeSDL2();

	if (!initialized) {
		cout << "Closing due to initialization errors.";
		exit(mainWindowSurface, mainWindow);
		return -1;
	}

	// Some of this shoulf go in startGame function
	// And there should be a GameState SINGLETON so I don't have to pass all these variables into the "startGame" function
	// It can be one global SINGLETON

	gameState = GamePhase::chooseDoor;

	// These can ONLY increment... and so they'll be encapsulated within the singleton, with no option of decrement or resetting!
	int yesSwitchWins = 0;
	int yesSwitchLosses = 0;
	int noSwitchWins = 0;
	int noSwitchLosses = 0;


	// Create three doors
	vector<Door> doors{ Door(), Door(), Door() };
	srand(time(0)); // This guarantees a NEW random number each time the rand() program runs

	// Timeout data
	const int TARGET_FPS = 60;
	const int FRAME_DELAY = 1000 / TARGET_FPS; // milliseconds per frame
	Uint32 frameStartTime; // Tick count when this particular frame began
	int frameTimeElapsed; // how much time has elapsed during this frame

	// ONCE THIS WORKS declare all the variables as NULL and then do NULL checks to cancel out in case of errors.
	mainWindow = SDL_CreateWindow("Main Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	mainRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);
	mainWindowSurface = SDL_GetWindowSurface(mainWindow);

	// Door Stuff
	SDL_Surface* doorSurface = IMG_Load("assets/door_400_400.png");
	SDL_Texture* doorTexture = SDL_CreateTextureFromSurface(mainRenderer, doorSurface);

	doorRects[0] = { getDoorHorizontalPosition(0), DOOR_Y_POSITION, DOOR_WIDTH, DOOR_HEIGHT };
	doorRects[1] = { getDoorHorizontalPosition(1), DOOR_Y_POSITION, DOOR_WIDTH, DOOR_HEIGHT };
	doorRects[2] = { getDoorHorizontalPosition(2), DOOR_Y_POSITION, DOOR_WIDTH, DOOR_HEIGHT };

	// title font stuff
	titleTextRect = { PADDING, PADDING, SCREEN_WIDTH - (PADDING * 2), 55 };
	std::string titleText = "Monty Hall Problem";
	SDL_Surface* titleTextSurface = TTF_RenderText_Solid(font, titleText.c_str(), textColor);
	// Create texture from surface pixels
	titleTextTexture = SDL_CreateTextureFromSurface(mainRenderer, titleTextSurface);
	SDL_FreeSurface(titleTextSurface);  // Free the surface after creating the texture

	// paint it white first (put this in a function later to avoid repeating the code during the game loop)
	SDL_SetRenderDrawColor(mainRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(mainRenderer);

	// Main loop flag
	bool running = true;

	//Event handler
	SDL_Event e;

	// Game loop
	while (running)
	{
		// Get the total running time (tick count) at the beginning of the frame for the frame timeout at the end
		frameStartTime = SDL_GetTicks();

		// Handle events in queue
		while (SDL_PollEvent(&e) != 0)
		{
			// User pressed X to close
			if (e.type == SDL_QUIT)
			{
				running = false;
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN) {
				// Clear window if user input happened
				SDL_SetRenderDrawColor(mainRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(mainRenderer);
				handleClick(&e);
			}
		}

		SDL_RenderCopyEx(mainRenderer, doorTexture, NULL, &doorRects[0], 0, NULL, SDL_FLIP_NONE);
		SDL_RenderCopyEx(mainRenderer, doorTexture, NULL, &doorRects[1], 0, NULL, SDL_FLIP_NONE);
		SDL_RenderCopyEx(mainRenderer, doorTexture, NULL, &doorRects[2], 0, NULL, SDL_FLIP_NONE);
		SDL_RenderCopyEx(mainRenderer, titleTextTexture, NULL, &titleTextRect, 0, NULL, SDL_FLIP_NONE);

		// Update window
		SDL_RenderPresent(mainRenderer);

		// Delay so the app doesn't just crash
		frameTimeElapsed = SDL_GetTicks() - frameStartTime; // Calculate how long the frame took to process
		// Delay loop
		if (frameTimeElapsed < FRAME_DELAY) {
			SDL_Delay(FRAME_DELAY - frameTimeElapsed);
		}
	}

	exit(mainWindowSurface, mainWindow);
	return 0;
}

bool initializeSDL2() {
	bool success = true;
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "SDL failed to initialize. SDL_Error: " << SDL_GetError() << std::endl;
		success = false;
	}

	// Initialize TTF
	if (TTF_Init() == -1) {
		std::cerr << "TTF failed to initialize. TTF_Error: " << TTF_GetError() << std::endl;
		success = false;
	}

	font = TTF_OpenFont("assets/Crang.ttf", 28);

	if (!font) {
		std::cerr << "Font failed to load. TTF_Error: " << TTF_GetError() << std::endl;
		success = false;
	}

	return success;
}


void handleClick(SDL_Event* e) {

	// Get location of click
	int x, y;
	SDL_GetMouseState(&x, &y);

	// display the info (delete later)
	cout << "\n\nMouse clicked";
	string xString = "\nX: " + std::to_string(x);
	string yString = "\nY: " + std::to_string(y);
	string printString = yString + xString;
	cout << printString;

	// see if it hit a button:

	if (y >= DOOR_Y_POSITION && y < DOOR_Y_POSITION + DOOR_HEIGHT) {
		cout << "\n\n CLICKED INSIDE THE BUTTON PLACE";

		// run through the doors and check it against their x/y

		for (int i = 0; i < 3; ++i) {
			if (x >= doorRects[i].x && x < (doorRects[i].x + DOOR_WIDTH)) {
				cout << "\n HIT DOOR " + std::to_string(i + 1) + '\n';
				// Set the draw color (RGBA)
				SDL_SetRenderDrawColor(mainRenderer, 30, 134, 214, 1);
				SDL_RenderFillRect(mainRenderer, &doorRects[i]);
			}
		}
	}
}


// Door manipulation functions ( get functions from MontyHallConsole )




int getDoorHorizontalPosition(int doorIndex) {
	return PADDING + ((PADDING + DOOR_WIDTH) * doorIndex);
}


void exit(SDL_Surface* surface, SDL_Window* window)
{
	SDL_FreeSurface(surface);
	surface = NULL;

	SDL_DestroyWindow(window);
	window = NULL;

	SDL_Quit();
}


