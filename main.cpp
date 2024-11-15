/*
* A small game to demonstate the Monty Hall Problem.
* There are three doors. One of them is the "winner" while the other two are duds.
* User picks one door. Then we open a "dud" from among the other two doors, and we open that door to show that it's a dud.
* Then user is prompted to either switch with the other unopened door, or hold onto their current selection.
* Finally we display the results. Is the user's selection the "winner" or a "dud" ?
* We also display cumulative results, because the whole point is to see whether
* switching or holding is statistically advantageous.
* 
* Game state (and stats) is held in a GameState object.
* Door objects hold information about the doors themselves (winner/dud ... open/closed ... chosen/not-chosen).
* GameState object operates on the doors.
* 
* main script accepts user input, displays GUI, and manages game loop.
* It's mostly concerned with SDL2 library.
*/


#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <SDL_ttf.h>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <time.h>

const bool DEBUG = false;

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
const int BUTTON_PADDING = 10;

const string titleText = "Monty Hall Problem";
const string loserText = "  DUD  ";
const string winnerText = "PRIZE";
const string mysteryText = "  ?  ";
const string chosenText = "CHOSEN";

const string instructionsText = "Choose a door!";
const string questionText = "Switch doors?";
const string buttonYesText = "YES";
const string buttonNoText = "NO";

// Stats strings
string yesWinsText = "";
string yesLossesText = "";
string noWinsText = "";
string noLossesText = "";

// function declarations
void exit(SDL_Surface* surface, SDL_Window* window);
int getDoorHorizontalPosition(int doorIndex);
void handleClick(SDL_Event* e);
bool initializeSDL2();
void draw();
void setWinsAndLossesTextures();
void printInstructions();
void setInstructionsText(string newText);
void setQuestionText(string newText);
void printQuestionAndButtons();
void printXY(int x, int y);
bool userClickedRect(SDL_Rect button, int x, int y);

SDL_Texture* doorTexture = NULL;
SDL_Texture* openDoorTexture = NULL;

SDL_Window* mainWindow = NULL;
SDL_Renderer* mainRenderer = NULL;
SDL_Surface* mainWindowSurface = NULL;

// font and text stuff
TTF_Font* font = NULL;
SDL_Color textColor = { 25, 25, 25 };
SDL_Color mysteryTextColor = { 250, 250, 50 };

SDL_Texture* titleTextTexture = NULL;
SDL_Texture* loserTextTexture = NULL;
SDL_Texture* winnerTextTexture = NULL;
SDL_Texture* mysteryTextTexture = NULL;
SDL_Texture* chosenDoorTextTexture = NULL;

// stats textures (to be updated after every GAME (win or loss))
SDL_Texture* yesWinsTextTexture = NULL;
SDL_Texture* yesLossesTextTexture = NULL;
SDL_Texture* noWinsTextTexture = NULL;
SDL_Texture* noLossesTextTexture = NULL;

SDL_Texture* buttonYesTexture = NULL;
SDL_Texture* buttonYesTextTexture = NULL;
SDL_Texture* buttonNoTexture = NULL;
SDL_Texture* buttonNoTextTexture = NULL;

SDL_Texture* questionTextTexture = NULL;
SDL_Texture* instructionsTextTexture = NULL;


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
SDL_Rect buttonYesRect;
SDL_Rect buttonYesTextRect;
SDL_Rect buttonNoRect;
SDL_Rect buttonNoTextRect;

SDL_Rect questionTextRect;
SDL_Rect chooseDoorRect;

SDL_Rect titleTextRect;

GameState gameState;

// Main loop flag
bool running = true;

/*
* Main function calls initializer (for libraries AND many textures).
* Also contains the main game loop.
* 
*/
int main(int argc, char* args[]) {

	if (!initializeSDL2()) {
		cout << "Closing due to initialization errors.";
		exit(mainWindowSurface, mainWindow);
		return -1;
	}

	gameState = GameState();

	// Timeout data
	const int TARGET_FPS = 60;
	const int FRAME_DELAY = 600 / TARGET_FPS; // milliseconds per frame
	Uint32 frameStartTime; // Tick count when this particular frame began
	int frameTimeElapsed; // how much time has elapsed during this frame

	// Draw once before the loop starts
	draw();

	//Event handler to store user input events for use within the game loop
	SDL_Event e;

	// Game loop
	while (running)
	{
		// Get the total running time (tick count) at the beginning of the frame, for the frame timeout at the end
		frameStartTime = SDL_GetTicks();

		// Check for events in queue, and handle them
		while (SDL_PollEvent(&e) != 0)
		{
			// User pressed X to close
			if (e.type == SDL_QUIT)
			{
				running = false;
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN) {
				// Everything happens in these two functions.
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

	// Initialize TTF font library
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

	// Load door images into surfaces
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

	// load door surfaces into usable textures
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

	// STATS rects
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

	questionTextRect = {
		PADDING,
		DOOR_Y_POSITION + DOOR_HEIGHT + PADDING + TEXT_HEIGHT + PADDING,
		(SCREEN_WIDTH / 2) - ((PADDING * 3) / 2),
		TEXT_HEIGHT
	};

	buttonYesRect = {
		(SCREEN_WIDTH / 2) + (PADDING / 2),
		questionTextRect.y,
		SCREEN_WIDTH / 4 - PADDING,
		TEXT_HEIGHT
	};

	buttonNoRect = {
		((SCREEN_WIDTH / 4) * 3) + (PADDING / 2),
		questionTextRect.y,
		SCREEN_WIDTH / 4 - (PADDING),
		TEXT_HEIGHT
	};

	buttonNoTextRect = {
		buttonNoRect.x + BUTTON_PADDING,
		buttonNoRect.y + BUTTON_PADDING,
		buttonNoRect.w - (BUTTON_PADDING * 2),
		buttonNoRect.h - (BUTTON_PADDING * 2)
	};

	buttonYesTextRect = {
		buttonYesRect.x + BUTTON_PADDING,
		buttonYesRect.y + BUTTON_PADDING,
		buttonYesRect.w - (BUTTON_PADDING * 2),
		buttonYesRect.h - (BUTTON_PADDING * 2)
	};

	// stats textures must be rebuilt after every game, so they get their own function.
	setWinsAndLossesTextures();

	// create text textures
	// create surfaces first (they die with scope) and then create textures from surface pixels
	SDL_Surface* titleTextSurface = TTF_RenderText_Blended(font, titleText.c_str(), textColor);
	titleTextTexture = SDL_CreateTextureFromSurface(mainRenderer, titleTextSurface);

	SDL_Surface* loserTextSurface = TTF_RenderText_Blended(font, loserText.c_str(), textColor);
	loserTextTexture = SDL_CreateTextureFromSurface(mainRenderer, loserTextSurface);

	SDL_Surface* winnerTextSurface = TTF_RenderText_Blended(font, winnerText.c_str(), textColor);
	winnerTextTexture = SDL_CreateTextureFromSurface(mainRenderer, winnerTextSurface);

	SDL_Surface* mysteryTextSurface = TTF_RenderText_Blended(font, mysteryText.c_str(), mysteryTextColor);
	mysteryTextTexture = SDL_CreateTextureFromSurface(mainRenderer, mysteryTextSurface);

	SDL_Surface* chosenTextSurface = TTF_RenderText_Blended(font, chosenText.c_str(), mysteryTextColor);
	chosenDoorTextTexture = SDL_CreateTextureFromSurface(mainRenderer, chosenTextSurface);

	// make switch question & button surfaces & textures (and other instruction surfaces and textures)
	SDL_Surface* buttonYesTextSurface = TTF_RenderText_Blended(font, buttonYesText.c_str(), textColor);
	buttonYesTextTexture = SDL_CreateTextureFromSurface(mainRenderer, buttonYesTextSurface);

	SDL_Surface* buttonNoTextSurface = TTF_RenderText_Blended(font, buttonNoText.c_str(), textColor);
	buttonNoTextTexture = SDL_CreateTextureFromSurface(mainRenderer, buttonNoTextSurface);
		
	// button question is updated based on game phase, so that texture is created in a function.
	setQuestionText(questionText);

	// instructions are updated based on game phase, so that texture is created in a function.
	setInstructionsText(instructionsText);

	// Free the surfaces after creating the textures
	SDL_FreeSurface(titleTextSurface);
	SDL_FreeSurface(loserTextSurface);
	SDL_FreeSurface(winnerTextSurface);
	SDL_FreeSurface(mysteryTextSurface);
	SDL_FreeSurface(buttonYesTextSurface);
	SDL_FreeSurface(buttonNoTextSurface);
	SDL_FreeSurface(chosenTextSurface);
	
	return true;
}


/*
* The game mechanics mostly happen here.
* User input changes the state of the gameState object.
* Then the draw() function draws new state.
*/
void handleClick(SDL_Event* e) {

	// Get location of click
	int x, y;
	SDL_GetMouseState(&x, &y);

	if (DEBUG) {
		printXY(x, y);
	}

	// see if mouse click hit a button.
	// Check game phase first, then check if ACTIVE buttons have been pressed.
	if (gameState.getGamePhase() == GamePhase::chooseDoor) {

		// See if user chose (clicked) a door.
		// run through the doors and check their area against x,y of mouse click
		for (int i = 0; i < 3; ++i) {
			if (userClickedRect(doorRects[i], x, y)) {
				// user clicked this doorRect
				gameState.userChoosesDoor(i);
				setInstructionsText("You chose door #" + to_string(i + 1));
			}
		}

	} else if(gameState.getGamePhase() == GamePhase::chooseSwitch) {
		bool userSwitches;
		bool buttonClicked = false;
		if (userClickedRect(buttonYesRect, x, y)) {
			userSwitches = true;
			buttonClicked = true;
		} else if (userClickedRect(buttonNoRect, x, y)) {
			userSwitches = false;
			buttonClicked = true;
		}

		if (buttonClicked) {
			bool userWins = gameState.chooseSwitchAndEndGame(userSwitches);
			string postGameInstructionsText = userWins ? "YOU WON!" : "YOU LOST!";
			setWinsAndLossesTextures();
			setInstructionsText(postGameInstructionsText);
			setQuestionText("Play again?");
		}
	}
	else if (gameState.getGamePhase() == GamePhase::gameOver) {
		// handle click on "reset game" button (doesn't exist yet)
		if (userClickedRect(buttonYesRect, x, y)) {
			gameState.resetGame();
			setInstructionsText(instructionsText);
		}
		else if (userClickedRect(buttonNoRect, x, y)) {
			running = false;
		}
	}
}

// find out if mouse click (x, y) happened within area of given SDL_Rect rectangle
bool userClickedRect(SDL_Rect button, int x, int y) {
	return y >= button.y &&
		y <= button.y + button.h &&
		x >= button.x &&
		x <= button.x + button.w;
}

// Based on game state draw everything (doors, buttons, text)
void draw() {
	// Clear window if user input happened
	SDL_SetRenderDrawColor(mainRenderer, 145, 145, 154, 1);
	SDL_RenderClear(mainRenderer);

	// Doors for game mechanics & data. Not the door images.
	vector<Door> doors = gameState.getDoors();

	// For each door, draw their backgrounds, text, the door image, and the question marks (where appropriate)
	for (int i = 0; i < doors.size(); ++i) {

		// Draw colored rectangles behind certain doors (open doors or chosen door)		
		if (doors[i].getChosen()) {
			// blue
			SDL_SetRenderDrawColor(mainRenderer, 95, 77, 227, 1);
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
				doors[i].getChosen() ? chosenDoorTextTexture : mysteryTextTexture,
				NULL,
				&doorTextRects[i],
				0,
				NULL,
				SDL_FLIP_NONE
			);
		}
	}

	printInstructions();
	// Make sure the correct text is rendered, depending on gamePhase
	if (gameState.getGamePhase() == GamePhase::chooseDoor) {
		setQuestionText(questionText);
		setInstructionsText(instructionsText);
	}
	if (gameState.getGamePhase() == GamePhase::chooseSwitch) {
		printInstructions();
		printQuestionAndButtons();
	}
	else if (gameState.getGamePhase() == GamePhase::gameOver) {
		printQuestionAndButtons();
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

void printQuestionAndButtons() {
	SDL_SetRenderDrawColor(mainRenderer, 255, 141, 0, 1);
	SDL_RenderFillRect(mainRenderer, &buttonYesRect);
	SDL_RenderFillRect(mainRenderer, &buttonNoRect);

	SDL_RenderCopyEx(mainRenderer, questionTextTexture, NULL, &questionTextRect, 0, NULL, SDL_FLIP_NONE);
	SDL_RenderCopyEx(mainRenderer, buttonYesTextTexture, NULL, &buttonYesTextRect, 0, NULL, SDL_FLIP_NONE);
	SDL_RenderCopyEx(mainRenderer, buttonNoTextTexture, NULL, &buttonNoTextRect, 0, NULL, SDL_FLIP_NONE);
}

void setInstructionsText(string newText) {
	SDL_Surface* chooseDoorSurface = TTF_RenderText_Blended(font, newText.c_str(), textColor);
	instructionsTextTexture = SDL_CreateTextureFromSurface(mainRenderer, chooseDoorSurface);
	SDL_FreeSurface(chooseDoorSurface);
}

void setQuestionText(string newText) {
	SDL_Surface* questionSurface = TTF_RenderText_Blended(font, newText.c_str(), textColor);
	questionTextTexture = SDL_CreateTextureFromSurface(mainRenderer, questionSurface);
	SDL_FreeSurface(questionSurface);
}

// For sizing purposes keep at least three digits in the stats counter by adding zeroes to the beginning.
string getThreeDigitCountString(int count) {
	string countString = to_string(count);
	while (countString.size() < 3) {
		countString = "0" + countString;
	}
	return countString;
}

// Stats textures
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

	// Free the surfaces after creating the textures
	SDL_FreeSurface(yesWinsTextSurface);
	SDL_FreeSurface(noWinsTextSurface);
	SDL_FreeSurface(yesLossesTextSurface);
	SDL_FreeSurface(noLossesTextSurface);
}


// Find out where this door should be placed horizontally
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
	cout << "\n\nMouse clicked at:";
	string xString = "\nX: " + std::to_string(x);
	string yString = "\nY: " + std::to_string(y);
	string printString = yString + xString;
	cout << printString;
}
