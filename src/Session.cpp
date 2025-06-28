#include "Session.h"
#include "utils.h"

void Session::newSession(string scrName, string timeCreated) {
	name = scrName;
	currentLine = "";
	timestamp = timeCreated;
}

string Session::GetName() {
	return name;
}

void Session::screen() {
	//Display session name and time created
	printColor(name + "\n\n", YELLOW);
	printColor(timestamp + "\n", YELLOW);
	cout << "Total commands happened on screen: " << totalLines << "\n\n";
	cout << "Previous command done: " << currentLine << "\n\n";

	printPlaceHolderConsoles();
	while (true) {
		std::string command;
		printColor("~> ", CYAN);
		std::getline(std::cin, command);
		if (command == "G") {
			printColor("Getting help \n", YELLOW);
			totalLines++;
			currentLine = command;
		}
		else if (command == "O") {
			printColor("Writing Out \n", YELLOW);
			totalLines++;
			currentLine = command;
		}
		else if (command == "R") {
			printColor("Reading File \n", YELLOW);
			totalLines++;
			currentLine = command;
		}
		else if (command == "Y") {
			printColor("I guess we use a go-to to traverse pages? \n", YELLOW);
			totalLines++;
			currentLine = command;
		}
		else if (command == "K") {
			printColor("Cutting Text \n", YELLOW);
			totalLines++;
			currentLine = command;
		}
		else if (command == "C") {
			printColor("Current Position \n", YELLOW);
			totalLines++;
			currentLine = command;
		}
		else if (command == "J") {
			printColor("Justifying \n", YELLOW);
			totalLines++;
			currentLine = command;
		}
		else if (command == "W") {
			printColor("Some Strcmp fucntion to search? \n", YELLOW);
			totalLines++;
			currentLine = command;
		}
		else if (command == "V") {
			printColor("Same as prev page, maybe a go-to to traverse? \n", YELLOW);
			totalLines++;
			currentLine = command;
		}
		else if (command == "U") {
			printColor("Tf does this even mean? Undo? \n", YELLOW);
			totalLines++;
			currentLine = command;
		}
		else if (command == "T") {
			printColor("Spelling \n", YELLOW);
			totalLines++;
			currentLine = command;
		}
		else if (command == "X") {
			break;
		}
		else {
			printColor("Unknown command\n", RED);
		}
	}
	system("cls");

	printBanner();
	printSubtitle();
}