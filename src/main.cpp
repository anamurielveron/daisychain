// daisychain.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <map> // for a hashmap
#include <iomanip>
#include <sstream>
#include <windows.h>

#include "utils.h"
#include "Process.h"
#include "FCFSScheduler.h"
#include "Session.h"

using namespace std;

// Forward declarations
class Process;
class FCFSScheduler;

// Global scheduler instance
FCFSScheduler* globalScheduler = nullptr;

/**
* SESSION CLASS
*/

//array of sessions for individual screen
Session sessions[10];

void initialize() {
	printColor("\"initialize\" command recognized. Doing something...\n", YELLOW);
	// TODO: Implement the initialize command
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


/**
* MAIN FUNCTION
*/
int main()
{
	int currentSessionCount = 0;

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
					if (command.substr(command.find("-s") + 2) != "" && command.substr(command.find("-s") + 3) != "") {
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
					else {
						printColor("Invalid screen name...\n", RED);
					}
				}
				else if (command.find("-r") != string::npos) {
					if (command.substr(command.find("-r") + 2) != "" && command.substr(command.find("-r") + 3) != "") {
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
						printColor("Invalid screen name...\n", RED);
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
		}
		break;
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
