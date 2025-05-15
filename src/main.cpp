// daisychain.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include <iostream>
#include <fstream>
#include <string>

#include "utils.h"

/**
* TEMPLATE COMMAND FUNCTIONS
*/

void initialize() {
	printColor("\"initialize\" command recognized. Doing something...\n", YELLOW);
	// TODO: Implement the initialize command
}

void screen() {
	printColor("\"screen\" command recognized. Doing something...\n", YELLOW);
	// TODO: Implement the screen command
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

int main()
{
	// Print welcome banner
	printBanner();
	printSubtitle();

	while (true) {
		std::string command;
		printColor("~> ", CYAN);
		std::getline(std::cin, command);

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
