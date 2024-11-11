module;
export module Door;


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