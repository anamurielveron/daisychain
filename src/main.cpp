// daisychain.cpp : This file contains the 'main' function. Program execution begins and ends there.


//#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
//#include <bits/stdc++.h>

#include <map> // for a hashmap
#include <iomanip>
#include <sstream>
#include <windows.h>

#include "utils.h"

using namespace std;

/**
* TEMPLATE COMMAND FUNCTIONS
*/

struct Session {
	string name;
	int currentLine;
	int totalLines;
	string timestamp;
};

void initialize() {
	printColor("\"initialize\" command recognized. Doing something...\n", YELLOW);
	// TODO: Implement the initialize command
}

void screen(Session currentSession) {
	//printColor("\"screen\" command recognized. Doing something...\n", YELLOW);
	// TODO: Implement the screen command
	printColor(currentSession.name + "\n", YELLOW);
	printColor(currentSession.timestamp + "\n", YELLOW);
	while (true) {
		std::string command;
		printColor("~> ", CYAN);
		std::getline(std::cin, command);
		if (command == "exit") {
			printColor("Leaving Screen...\n", RED);
			break;
		}
	}
	system("cls");

	printBanner();
	printSubtitle();
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
	Session sessions[10];

	int currentSessionCount = 0;

	int sessionToResume = 0;

	bool screenFound = false;
	// Print welcome banner
	while (true) 
	{
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
					//system("cls");
					//screen();

					//std::cout << "Creating new screen \"" << command.substr(command.find("-s") + 3) << "\"...\n";
					for (Session session : sessions) {
						if (session.name == command.substr(command.find("-s") + 3)) {
							screenFound = true;
							break;
						}
					}
					if (!screenFound) {
						sessions[currentSessionCount] = { command.substr(command.find("-s") + 3), 0, 0, getCurrentTimestamp() };
						system("cls");
						screen(sessions[currentSessionCount]);
						currentSessionCount++;
					}
					else {
						printColor("Screen already exists...\n", RED);
					}
				}
				else if (command.find("-r") != string::npos) {
					sessionToResume = 0;
					for (Session session : sessions) {
						if (session.name == command.substr(command.find("-r") + 3)) {
							screenFound = true;
							break;
						}
						sessionToResume++;
					}

					if (screenFound) {
						system("cls");
						screen(sessions[sessionToResume]);
					}
					else {
						printColor("Screen does not exist...\n", RED);
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
