// daisychain.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include <iostream>
#include <fstream>
#include <string>

#include <map> // for a hashmap
#include <iomanip>
#include <sstream>
#include <windows.h>

#include "utils.h"

using namespace std;

/**
* TEMPLATE COMMAND FUNCTIONS
*/

<<<<<<< Updated upstream
=======
// Session struct to hold screen info
class Session {
	string name;
	string currentLine;
	int totalLines=0;
	string timestamp;

	public:
		void newSession(string scrName, string timeCreated) {
			name = scrName;
			currentLine = "";
			timestamp = timeCreated;
		}

		void screen();

		string GetName() {
			return name;
		}
};

Session sessions[10];

>>>>>>> Stashed changes
void initialize() {
	printColor("\"initialize\" command recognized. Doing something...\n", YELLOW);
	// TODO: Implement the initialize command
}

<<<<<<< Updated upstream
void screen() {
	printColor("\"screen\" command recognized. Doing something...\n", YELLOW);
	// TODO: Implement the screen command
=======
void Session::screen() {
	//printColor("\"screen\" command recognized. Doing something...\n", YELLOW);
	// TODO: Implement the screen command

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
		if (command == "X") {
			printColor("Leaving Screen...\n", RED);
			break;
		}
		else {
			printColor("Doing something \n", YELLOW);
			totalLines++;
			currentLine = command;
		}
	}
	system("cls");

	printBanner();
	printSubtitle();
>>>>>>> Stashed changes
}

void schedulerTest() {
	printColor("\"scheduler-test\" command recognized. Doing something...\n", YELLOW);
	// TODO: Implement the scheduler-test command
}

void schedulerStop() {
	printColor("\"scheduler-stop\" command recognized. Doing something...\n", YELLOW);
	// TODO: Implement the scheduler-stop command
}

void reportUtil() {
	printColor("\"report-util\" command recognized. Doing something...\n", YELLOW);
	// TODO: Implement the report-util command
}

// Session struct to hold screen info
struct Session {
	string name;
	int currentLine;
	int totalLines;
	string timestamp;
};


string getCurrentTimestamp() {
	SYSTEMTIME st;
	GetLocalTime(&st);
	char buffer[100]; // to hold the time

	//formats the time to 12 hour
	string am_pm = (st.wHour >= 12) ? "PM" : "AM";
	int hour = st.wHour % 12;
	if (hour == 0) hour = 12;

	sprintf_s(buffer, "%02d/%02d/%04d, %02d:%02d:%02d %s",
		st.wMonth, st.wDay, st.wYear,
		hour, st.wMinute, st.wSecond,
		am_pm.c_str());

	return string(buffer);
}


/**
* MAIN FUNCTION
*/
int main()
{
	// Print welcome banner
	printBanner();
	printSubtitle();

<<<<<<< Updated upstream
	while (true) {
		std::string command;
		printColor("~> ", CYAN);
		std::getline(std::cin, command);
=======
	//Holds the index for which session to resume
	int sessionToResume = 0;

	bool screenFound = false;
	
	while (true) 
	{
		// Print welcome banner
		printBanner();
		printSubtitle();

		while (true) {
			screenFound = false;
			std::string command;
			printColor("~> ", CYAN);
			std::getline(std::cin, command);

			if (command == "help") {
				printHelp();
			}
			else if (command == "initialize") {
				initialize();
			}
			else if (command.find("screen") != string::npos) {
				if (command.find("-s") != string::npos) {
					
					//Checks if session name already exists
					for (Session session : sessions) {
						if (session.GetName() == command.substr(command.find("-s") + 3)) {
							screenFound = true;
							break;
						}
					}
					
					if (!screenFound) {
						//If session name does not exist, create new session
						sessions[currentSessionCount].newSession(command.substr(command.find("-s") + 3), getCurrentTimestamp());
						system("cls");
						sessions[currentSessionCount].screen();
						currentSessionCount++;
					}
					else {
						printColor("Screen already exists...\n", MAGENTA);
					}
				}
				else if (command.find("-r") != string::npos) {
					sessionToResume = 0;

					//Checks if session name already exists
					for (Session session : sessions) {
						if (session.GetName() == command.substr(command.find("-r") + 3)) {
							screenFound = true;
							break;
						}
						sessionToResume++;
					}

					if (screenFound) {
						//If session name exists, resume session
						system("cls");
						sessions[sessionToResume].screen();
					}
					else {
						printColor("Screen does not exist...\n", MAGENTA);
					}
				}
				else {
					printColor("Screen command not recognized....\n", RED);
				}
			}
			else if (command == "scheduler-test") {
				schedulerTest();
			}
			else if (command == "scheduler-stop") {
				schedulerStop();
			}
			else if (command == "report-util") {
				reportUtil();
			}
			else if (command == "clear") {
				clear();
			}
			else if (command == "exit") {
				printColor("Exiting...\n", RED);
				break;
			}
			else {
				printColor("Unknown command. Type 'help' for a list of commands.\n", RED);
			}
>>>>>>> Stashed changes

		if (command == "help") {
			printHelp();
		}
		else if (command == "initialize") {
			initialize();
		}
		else if (command == "screen") {
			screen();
		}
		else if (command == "scheduler-test") {
			schedulerTest();
		}
		else if (command == "scheduler-stop") {
			schedulerStop();
		}
		else if (command == "report-util") {
			reportUtil();
		}
		else if (command == "clear") {
			clear();
		}
		else if (command == "exit") {
			printColor("Exiting...\n", RED);
			break;
		}
		else {
			printColor("Unknown command. Type 'help' for a list of commands.\n", RED);
		}

	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
