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
*		- Remove double padding between elements
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
* 8. Save colors as variables (blue for chosen + closed, red for losing + open, green for winning + open)
*
* IMAGE DISPLAYED SUCCESSFULLY!
* 
* NEXT:
*	- Print instructions
*		- The instructions should be able to CHANGE over time.
*	- Print stats (print STATE)
*		- It should print EVERYTHING each time something CHANGES, but EVERYTHING should be based on state. (don't just print once and leave it until there's a change).
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

const bool DEBUG = true;

import GameState;

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
const int DOOR_Y_POSITION = DOOR_HEIGHT * 1.25;
const int TEXT_HEIGHT = 54;
const int STATS_TEXT_HEIGHT = 34;
const int SWITCH_BUTTON_WIDTH = 90;
const int SWITCH_BUTTON_HEIGHT = 30;
const int SWITCH_BUTTON_PADDING = 10;

const string loserText = " LOSER";
const string winnerText = "WINNER";
const string titleText = "Monty Hall Problem";
const string mysteryText = "  ?  ";

string instructionsText = "Choose a door!";

const string switchQuestionText = "Switch doors?";
const string switchButtonYesText = "YES";
const string switchButtonNoText = "NO";

string yesWinsText = "";
string yesLossesText = "";
string noWinsText = "";
string noLossesText = "";

void exit(SDL_Surface* surface, SDL_Window* window);

int getDoorHorizontalPosition(int doorIndex);
void handleClick(SDL_Event* e);
bool initializeSDL2();
void draw();
void setWinsAndLossesTextures();
void printInstructions();
void setInstructionsText(string newText);

SDL_Texture* doorTexture = NULL;
SDL_Texture* openDoorTexture = NULL;

SDL_Window* mainWindow = NULL;
SDL_Renderer* mainRenderer = NULL;
SDL_Surface* mainWindowSurface = NULL;

// font stuff
TTF_Font* font = NULL;
SDL_Color textColor = { 25, 25, 25 };
SDL_Color mysteryTextColor = { 250, 250, 50 };
SDL_Rect titleTextRect;
SDL_Texture* titleTextTexture = NULL;
SDL_Texture* loserTextTexture = NULL;
SDL_Texture* winnerTextTexture = NULL;
SDL_Texture* mysteryTextTexture = NULL;

// Update these (stats) after every GAME (win or loss)
SDL_Texture* yesWinsTextTexture = NULL;
SDL_Texture* yesLossesTextTexture = NULL;
SDL_Texture* noWinsTextTexture = NULL;
SDL_Texture* noLossesTextTexture = NULL;

SDL_Texture* switchButtonYesTexture;
SDL_Texture* switchButtonYesTextTexture;
SDL_Texture* switchButtonNoTexture;
SDL_Texture* switchButtonNoTextTexture;

SDL_Texture* switchQuestionTextTexture;
SDL_Texture* instructionsTextTexture;


// Rectangles

// rectangles to display the doors and the WINNER and LOSER text behind the doors
SDL_Rect doorRects[3];
SDL_Rect doorTextRects[3];

// stats rectangles
SDL_Rect yesWinsTextRect;
SDL_Rect yesLossesTextRect;
SDL_Rect noWinsTextRect;
SDL_Rect noLossesTextRect;

// Switch button Rectangle
SDL_Rect switchButtonYesRect;
SDL_Rect switchButtonYesTextRect;
SDL_Rect switchButtonNoRect;
SDL_Rect switchButtonNoTextRect;

SDL_Rect switchQuestionTextRect;
SDL_Rect chooseDoorRect;


GameState gameState;

void printXY(int x, int y);


int main(int argc, char* args[]) {
	cout << "THis is the main function";

	bool initialized = initializeSDL2();

	if (!initialized) {
		cout << "Closing due to initialization errors.";
		exit(mainWindowSurface, mainWindow);
		return -1;
	}

	gameState = GameState();

	cout << "Number of Doors:\n";
	cout << gameState.getDoors().size();
	cout << '\n';

	cout << "Winning Door Index:\n";
	cout << gameState.getWinningDoorIndex();
	cout << '\n';

	// Some of this shoulf go in startGame function
	// And there should be a GameState SINGLETON so I don't have to pass all these variables into the "startGame" function
	// It can be one global SINGLETON


	// Create three doors // NO... the doors will be in GameState
	//vector<Door> doors{ Door(), Door(), Door() };
	//srand(time(0)); // This guarantees a NEW random number each time the rand() program runs

	// Timeout data
	const int TARGET_FPS = 60;
	const int FRAME_DELAY = 1000 / TARGET_FPS; // milliseconds per frame
	Uint32 frameStartTime; // Tick count when this particular frame began
	int frameTimeElapsed; // how much time has elapsed during this frame

	// Draw once before the loop starts
	draw();

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
				handleClick(&e);
				draw();
			}
		}

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

// Load the SDL libraries, plus the main textures and rectangles
bool initializeSDL2() {
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("SDL failed to initialize. SDL_Error: %s\n", SDL_GetError());
		std::cerr << "SDL failed to initialize. SDL_Error: " << SDL_GetError() << std::endl;
		return false;
	}

	// Initialize TTF
	if (TTF_Init() == -1) {
		SDL_Log("WTTF failed to initialize. TTF_Error: %s\n", TTF_GetError());
		std::cerr << "TTF failed to initialize. TTF_Error: " << TTF_GetError() << std::endl;
		return false;
	}

	font = TTF_OpenFont("assets/Crang.ttf", 28);

	if (!font) {
		SDL_Log("Font failed to load. TTF_Error: %s\n", TTF_GetError());
		std::cerr << "Font failed to load. TTF_Error: " << TTF_GetError() << std::endl;
		return false;
	}

	// Load main window, surface, and renderer
	mainWindow = SDL_CreateWindow("Main Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

	if (!mainWindow) {
		SDL_Log("Window failed to load. SDL_Error: %s\n", SDL_GetError());
		std::cerr << "Window failed to load. SDL_Error: " << SDL_GetError() << std::endl;
		return false;
	}

	mainRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);

	if (!mainRenderer) {
		SDL_Log("Renderer failed to load. SDL_Error: %s\n", SDL_GetError());
		std::cerr << "Renderer failed to load. SDL_Error: " << SDL_GetError() << std::endl;
		return false;
	}

	mainWindowSurface = SDL_GetWindowSurface(mainWindow);

	if (!mainWindowSurface) {
		SDL_Log("Window Surface failed to load. SDL_Error: %s\n", SDL_GetError());
		std::cerr << "Window Surface failed to load. SDL_Error: " << SDL_GetError() << std::endl;
		return false;
	}

	// Door Stuff
	SDL_Surface* doorSurface = IMG_Load("assets/door_400_400.png");

	if (!doorSurface) {
		std::cerr << "Image failed to load. SDL_image: " << IMG_GetError() << std::endl;
		return false;
	}

	SDL_Surface* openDoorSurface = IMG_Load("assets/open_door_400_400.png");

	if (!openDoorSurface) {
		std::cerr << "Image failed to load. SDL_image: " << IMG_GetError() << std::endl;
		return false;
	}

	doorTexture = SDL_CreateTextureFromSurface(mainRenderer, doorSurface);
	openDoorTexture = SDL_CreateTextureFromSurface(mainRenderer, openDoorSurface);

	// Creating Rects

	// Door rectangles AND door text rectangles
	for (int i = 0; i < 3; ++i) {
		doorRects[i] = { getDoorHorizontalPosition(i), DOOR_Y_POSITION, DOOR_WIDTH, DOOR_HEIGHT };
		doorTextRects[i] = {
			getDoorHorizontalPosition(i) + ((PADDING * 3) / 2),
			(DOOR_Y_POSITION + (DOOR_HEIGHT / 2)) - (TEXT_HEIGHT / 2),
			DOOR_WIDTH - (PADDING * 3),
			TEXT_HEIGHT
		};
	}

	titleTextRect = { PADDING, PADDING, SCREEN_WIDTH - (PADDING * 2), TEXT_HEIGHT };

	// STATS rects (stats textures must be rebuilt after every game, so they get their own function).
	//top left (of stats area)
	yesWinsTextRect = {
		PADDING,
		(PADDING * 3) + TEXT_HEIGHT,
		(SCREEN_WIDTH / 2) - (PADDING * 2),
		STATS_TEXT_HEIGHT };
	// top right
	noWinsTextRect = {
		PADDING,
		(PADDING * 3) + (TEXT_HEIGHT * 2),
		(SCREEN_WIDTH / 2) - (PADDING * 2),
		STATS_TEXT_HEIGHT };
	// bottom left
	yesLossesTextRect = {
		PADDING + (SCREEN_WIDTH / 2),
		(PADDING * 3) + TEXT_HEIGHT,
		(SCREEN_WIDTH / 2) - (PADDING * 2),
		STATS_TEXT_HEIGHT };
	// bottom right
	noLossesTextRect = {
		PADDING + (SCREEN_WIDTH / 2),
		(PADDING * 3) + (TEXT_HEIGHT * 2),
		(SCREEN_WIDTH / 2) - (PADDING * 2),
		STATS_TEXT_HEIGHT };

	// Switch Button Stuff
	chooseDoorRect = {
		PADDING,
		DOOR_Y_POSITION + DOOR_HEIGHT + PADDING,
		(SCREEN_WIDTH / 2) - (PADDING * 2),
		TEXT_HEIGHT
	};

	switchQuestionTextRect = {
		PADDING,
		DOOR_Y_POSITION + DOOR_HEIGHT + PADDING + TEXT_HEIGHT + PADDING,
		(SCREEN_WIDTH / 2) - (PADDING * 2),
		TEXT_HEIGHT
	};

	switchButtonYesRect = {
		(SCREEN_WIDTH / 2) + PADDING,
		switchQuestionTextRect.y,
		SCREEN_WIDTH / 4 - (PADDING * 2),
		TEXT_HEIGHT
	};

	switchButtonNoRect = {
		((SCREEN_WIDTH / 4) * 3) + PADDING,
		switchQuestionTextRect.y,
		SCREEN_WIDTH / 4 - (PADDING * 2),
		TEXT_HEIGHT
	};

	switchButtonNoTextRect = {
		switchButtonNoRect.x + SWITCH_BUTTON_PADDING,
		switchButtonNoRect.y + SWITCH_BUTTON_PADDING,
		switchButtonNoRect.w - (SWITCH_BUTTON_PADDING * 2),
		switchButtonNoRect.h - (SWITCH_BUTTON_PADDING * 2)
	};

	switchButtonYesTextRect = {
		switchButtonYesRect.x + SWITCH_BUTTON_PADDING,
		switchButtonYesRect.y + SWITCH_BUTTON_PADDING,
		switchButtonYesRect.w - (SWITCH_BUTTON_PADDING * 2),
		switchButtonYesRect.h - (SWITCH_BUTTON_PADDING * 2)
	};

	setWinsAndLossesTextures();

	// create text textures
	// create surfaces first (dies with scope) and then create textures from surface pixels
	SDL_Surface* titleTextSurface = TTF_RenderText_Blended(font, titleText.c_str(), textColor);
	titleTextTexture = SDL_CreateTextureFromSurface(mainRenderer, titleTextSurface);

	SDL_Surface* loserTextSurface = TTF_RenderText_Blended(font, loserText.c_str(), textColor);
	loserTextTexture = SDL_CreateTextureFromSurface(mainRenderer, loserTextSurface);

	SDL_Surface* winnerTextSurface = TTF_RenderText_Blended(font, winnerText.c_str(), textColor);
	winnerTextTexture = SDL_CreateTextureFromSurface(mainRenderer, winnerTextSurface);

	SDL_Surface* mysteryTextSurface = TTF_RenderText_Blended(font, mysteryText.c_str(), mysteryTextColor);
	mysteryTextTexture = SDL_CreateTextureFromSurface(mainRenderer, mysteryTextSurface);

	// make switch question & button surfaces & textures (and other instruction surfaces and textures)
	SDL_Surface* switchButtonYesTextSurface = TTF_RenderText_Blended(font, switchButtonYesText.c_str(), textColor);
	switchButtonYesTextTexture = SDL_CreateTextureFromSurface(mainRenderer, switchButtonYesTextSurface);

	SDL_Surface* switchButtonNoTextSurface = TTF_RenderText_Blended(font, switchButtonNoText.c_str(), textColor);
	switchButtonNoTextTexture = SDL_CreateTextureFromSurface(mainRenderer, switchButtonNoTextSurface);

	SDL_Surface* switchQuestionSurface = TTF_RenderText_Blended(font, switchQuestionText.c_str(), textColor);
	switchQuestionTextTexture = SDL_CreateTextureFromSurface(mainRenderer, switchQuestionSurface);

	// instructions will need to be updated, so that texture is created in a function.
	setInstructionsText(instructionsText);

	// Free the surface after creating the texture
	SDL_FreeSurface(titleTextSurface);
	SDL_FreeSurface(loserTextSurface);
	SDL_FreeSurface(winnerTextSurface);
	SDL_FreeSurface(mysteryTextSurface);
	SDL_FreeSurface(switchButtonYesTextSurface);
	SDL_FreeSurface(switchButtonNoTextSurface);
	SDL_FreeSurface(switchQuestionSurface);	

	return true;
}


void handleClick(SDL_Event* e) {

	// Get location of click
	int x, y;
	SDL_GetMouseState(&x, &y);

	if (DEBUG) {
		printXY(x, y);
	}

	// see if it hit a button. Check game phase first, then check if ACTIVE buttons have been pressed.
	if (gameState.getGamePhase() == GamePhase::chooseDoor) {
		if (y >= DOOR_Y_POSITION && y < DOOR_Y_POSITION + DOOR_HEIGHT) {
			cout << "\n\n CLICKED INSIDE THE BUTTON PLACE";

			// run through the doors and check it against their x/y
			for (int i = 0; i < 3; ++i) {
				if (x >= doorRects[i].x && x < (doorRects[i].x + DOOR_WIDTH)) {
					//cout << "\n HIT DOOR " + std::to_string(i + 1) + '\n';

					gameState.chooseDoor(i);
					// Set the draw color (RGBA)
					SDL_SetRenderDrawColor(mainRenderer, 30, 134, 214, 1);
					SDL_RenderFillRect(mainRenderer, &doorRects[i]);
					setInstructionsText("You chose door #" + to_string(i + 1));
				}
			}
		}
	} else if(gameState.getGamePhase() == GamePhase::chooseSwitch) {
		bool userSwitches;;
		if (
			// clicked YES button
			y >= switchButtonYesRect.y &&
			y <= switchButtonYesRect.y + switchButtonYesRect.h &&
			x >= switchButtonYesRect.x &&
			x <= switchButtonYesRect.x + switchButtonYesRect.w) {
				userSwitches = true;

		} else if (
			// clicked NO button
			y >= switchButtonNoRect.y &&
			y <= switchButtonNoRect.y + switchButtonNoRect.h &&
			x >= switchButtonNoRect.x &&
			x <= switchButtonNoRect.x + switchButtonNoRect.w) {
				userSwitches = false;
		}
		bool userWins = gameState.chooseSwitchAndEndGame(userSwitches);
		string postGameInstructionsText = userWins ? "YOU WON!" : "YOU LOST!";
		setWinsAndLossesTextures();
		setInstructionsText(postGameInstructionsText);
	}
	else if (gameState.getGamePhase() == GamePhase::gameOver) {
		// handle click on "reset game" button (doesn't exist yet)
	}
}


void draw() {
	// Clear window if user input happened
	SDL_SetRenderDrawColor(mainRenderer, 104, 104, 104, 1);
	SDL_RenderClear(mainRenderer);

	vector<Door> doors = gameState.getDoors();

	// For each door, draw their backgrounds, text,  the door image, and the question marks (where appropriate)
	for (int i = 0; i < doors.size(); ++i) {

		// Draw colored rectangles behind certain doors (open doors or chosen door)		
		if (doors[i].getChosen()) {
			// blue
			SDL_SetRenderDrawColor(mainRenderer, 70, 40, 200, 1);
			SDL_RenderFillRect(mainRenderer, &doorRects[i]);
		} else if (doors[i].getOpen() && !doors[i].getWinner()) {
			SDL_SetRenderDrawColor(mainRenderer, 220, 34, 34, 1);
			SDL_RenderFillRect(mainRenderer, &doorRects[i]);
		}

		// draw WINNER or LOSER:
		SDL_RenderCopyEx(
			mainRenderer,
			doors[i].getWinner() ? winnerTextTexture : loserTextTexture,
			NULL,
			&doorTextRects[i],
			0,
			NULL,
			SDL_FLIP_NONE
		);

		// draw the doors
		SDL_RenderCopyEx(
			mainRenderer,
			gameState.getDoors()[i].getOpen() ? openDoorTexture : doorTexture,
			NULL,
			&doorRects[i],
			0,
			NULL,
			SDL_FLIP_NONE
		);

		// draw question marks on unopened doors IF it's the "should I switch" phase.
		if (gameState.getGamePhase() == GamePhase::chooseSwitch && !doors[i].getOpen()) {
			SDL_RenderCopyEx(
				mainRenderer,
				mysteryTextTexture,
				NULL,
				&doorTextRects[i],
				0,
				NULL,
				SDL_FLIP_NONE
			);
		}
	}
	printInstructions();
	if (gameState.getGamePhase() == GamePhase::chooseSwitch) {
		printInstructions();
		// Draw switch question items
		SDL_SetRenderDrawColor(mainRenderer, 255, 141, 0, 1);
		SDL_RenderFillRect(mainRenderer, &switchButtonYesRect);
		SDL_RenderFillRect(mainRenderer, &switchButtonNoRect);

		SDL_RenderCopyEx(mainRenderer, switchQuestionTextTexture, NULL, &switchQuestionTextRect, 0, NULL, SDL_FLIP_NONE);
		SDL_RenderCopyEx(mainRenderer, switchButtonYesTextTexture, NULL, &switchButtonYesTextRect, 0, NULL, SDL_FLIP_NONE);
		SDL_RenderCopyEx(mainRenderer, switchButtonNoTextTexture, NULL, &switchButtonNoTextRect, 0, NULL, SDL_FLIP_NONE);
	}
	else if (gameState.getGamePhase() == GamePhase::gameOver) {
		cout << "\nGAME OVER\n";
		bool userIsWinner = gameState.getWinningDoorIndex() == gameState.getChosenDoorIndex();
		string winnerOrLoserText = userIsWinner ? "WINNER!" : "LOSER!";
		cout << winnerOrLoserText;
	}


	// draw stats
	SDL_RenderCopyEx(mainRenderer, yesWinsTextTexture, NULL, &yesWinsTextRect, 0, NULL, SDL_FLIP_NONE);
	SDL_RenderCopyEx(mainRenderer, noWinsTextTexture, NULL, &noWinsTextRect, 0, NULL, SDL_FLIP_NONE);
	SDL_RenderCopyEx(mainRenderer, yesLossesTextTexture, NULL, &yesLossesTextRect, 0, NULL, SDL_FLIP_NONE);
	SDL_RenderCopyEx(mainRenderer, noLossesTextTexture, NULL, &noLossesTextRect, 0, NULL, SDL_FLIP_NONE);

	// draw title
	SDL_RenderCopyEx(mainRenderer, titleTextTexture, NULL, &titleTextRect, 0, NULL, SDL_FLIP_NONE);

	// Update window
	SDL_RenderPresent(mainRenderer);
}

void printInstructions() {
	SDL_RenderCopyEx(mainRenderer, instructionsTextTexture, NULL, &chooseDoorRect, 0, NULL, SDL_FLIP_NONE);
}

void setInstructionsText(string newText) {
	SDL_Surface* chooseDoorSurface = TTF_RenderText_Blended(font, newText.c_str(), textColor);
	instructionsTextTexture = SDL_CreateTextureFromSurface(mainRenderer, chooseDoorSurface);
	SDL_FreeSurface(chooseDoorSurface);
}

string getThreeDigitCountString(int count) {
	string countString = to_string(count);
	while (countString.size() < 3) {
		countString = "0" + countString;
	}
	return countString;
}


void setWinsAndLossesTextures() {

	// Adding extra spaces to keep characters from stretching.
	yesWinsText = "SWITCH WINS:              " + getThreeDigitCountString(gameState.getYesSwitchWins());
	noWinsText = "NON-SWITCH WINS:    " + getThreeDigitCountString(gameState.getNoSwitchWins());
	yesLossesText = "SWITCH LOSSES:           " + getThreeDigitCountString(gameState.getYesSwitchLosses());
	noLossesText = "NON-SWITCH LOSSES:  " + getThreeDigitCountString(gameState.getNoSwitchLosses());

	SDL_Surface* yesWinsTextSurface = TTF_RenderText_Blended(font, yesWinsText.c_str(), textColor);
	yesWinsTextTexture = SDL_CreateTextureFromSurface(mainRenderer, yesWinsTextSurface);

	SDL_Surface* noWinsTextSurface = TTF_RenderText_Blended(font, noWinsText.c_str(), textColor);
	noWinsTextTexture = SDL_CreateTextureFromSurface(mainRenderer, noWinsTextSurface);

	SDL_Surface* yesLossesTextSurface = TTF_RenderText_Blended(font, yesLossesText.c_str(), textColor);
	yesLossesTextTexture = SDL_CreateTextureFromSurface(mainRenderer, yesLossesTextSurface);

	SDL_Surface* noLossesTextSurface = TTF_RenderText_Blended(font, noLossesText.c_str(), textColor);
	noLossesTextTexture = SDL_CreateTextureFromSurface(mainRenderer, noLossesTextSurface);

	// Free the surface after creating the texture
	SDL_FreeSurface(yesWinsTextSurface);
	SDL_FreeSurface(noWinsTextSurface);
	SDL_FreeSurface(yesLossesTextSurface);
	SDL_FreeSurface(noLossesTextSurface);
}


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


// DEBUG FUNCTIONS

// display mouse click info
void printXY(int x, int y) {
	cout << "\n\nMouse clicked";
	string xString = "\nX: " + std::to_string(x);
	string yString = "\nY: " + std::to_string(y);
	string printString = yString + xString;
	cout << printString;
}
