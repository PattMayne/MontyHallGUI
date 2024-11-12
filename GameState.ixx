module;
export module GameState;

#include<vector>
#include <cstdlib> // Needed for rand() and srand()
#include <ctime>   // Needed for time()
#include<cstdlib>

using namespace std;
using std::vector;

export enum class GamePhase {
	chooseDoor, chooseSwitch, gameOver
};

export class Door {

	private:
		bool isWinner;
		bool isOpen;
		bool isChosen;

	public:
		Door() {
			isWinner = false;
			isOpen = false;
			isChosen = false;
		}

		void open() {
			isOpen = true;
		}

		void setWinner() {
			isWinner = true;
		}

		void choose() {
			isChosen = true;
		}

		void unchoose() {
			isChosen = false;
		}

		bool getWinner() {
			return isWinner;
		}

		bool getOpen() {
			return isOpen;
		}

		bool getChosen() {
			return isChosen;
		}
};

export class GameState {
	private:
		//vector<Door> doors{ Door(), Door(), Door() };
		vector<Door> doors;
		GamePhase gamePhase;

		// These can ONLY increment... and so they'll be encapsulated within the singleton, with no option of decrement or resetting!
		int yesSwitchWins;
		int yesSwitchLosses;
		int noSwitchWins;
		int noSwitchLosses;

		void setWinner() {
			doors[rand() % 3].setWinner();
		}

		vector<int> getLosingDoorIndices() {
			vector<int> losingDoorIndices;

			for (int i = 0; i < doors.size(); i++) {
				if (!doors[i].getWinner()) {
					losingDoorIndices.push_back(i);
				}
			}
			return losingDoorIndices;
		}

		void openOneLosingDoor() {
			vector<int> losingDoorIndices = getLosingDoorIndices();

			// check if user chose winner on the first selection
			bool userChoseWinner = false;
			for (Door door : doors) {
				if (door.getChosen() && door.getWinner()) {
					userChoseWinner = true;
				}
			}
			// if user chose winner, randomly select among the losing doors to open one.
			// If user chose loser, simply open the OTHER losing door.
			if (userChoseWinner) {
				const int losingDoorIndexToOpen = rand() % losingDoorIndices.size();
				doors[losingDoorIndices[losingDoorIndexToOpen]].open();
			}
			else {
				// find the losing door that's NOT chosen, and open it
				if (!doors[losingDoorIndices[0]].getChosen()) {
					doors[losingDoorIndices[0]].open();
				}
				else {
					doors[losingDoorIndices[1]].open();
				}
			}
		}

		void unchooseAllDoors() {
			for (int i = 0; i < 3; ++i) {
				doors[i].unchoose();
			}
		}

		void setupGame() {
			gamePhase = GamePhase::chooseDoor;
			doors = { Door(), Door(), Door() };
			setWinner();
		}


	public:
		vector<Door> getDoors() {
			return doors;
		}

		GameState() {
			// what happens when I put this in the constructor? Should I pass it in through a parameter instead? Pass what in?
			srand(time(0)); // This guarantees a NEW random number each time the rand() program runs
			setupGame();

			// These don't change during the program's run.
			// So we can play the game multiple times and see the cumulative stats.
			yesSwitchWins = 0;
			yesSwitchLosses = 0;
			noSwitchWins = 0;
			noSwitchLosses = 0;
		}

		void chooseDoor(int chosenDoorIndex) {
			unchooseAllDoors();
			doors[chosenDoorIndex].choose();
			gamePhase = GamePhase::chooseSwitch;
		}

		int getYesSwitchWIns() {
			return yesSwitchWins;
		}

		int getYesSwitchLosses() {
			return yesSwitchLosses;
		}

		int getNoSwitchWins() {
			return noSwitchWins;
		}

		int getNoSwitchLosses() {
			return noSwitchLosses;
		}

		GamePhase getGamePhase() {
			return gamePhase;
		}

		// Not sure yet how we'll increment wins and losses.

		// Door state will be stored in the doors themselves

		int getWinningDoorIndex() {
			for (int i = 0; i < doors.size(); i++) {
				if (doors[i].getWinner()) {
					return i;
				}
			}
			return -1;
		}

		int getChosenDoorIndex() {
			for (int i = 0; i < doors.size(); i++) {
				if (doors[i].getChosen()) {
					return i;
				}
			}
			return -1;
		}

		void resetGame() {
			setupGame();
		}
		
};

