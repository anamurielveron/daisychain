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
#include <shlobj.h> // For SHGetFolderPath

#include "utils.h"

using namespace std;

/**
* TEMPLATE COMMAND FUNCTIONS
*/

// Session struct to hold screen info
struct Session {
	string name;
	string currentLine;
	int totalLines;
	string timestamp;
};

// 14/06/2025 
#include <queue>

class Process {
	int id;
	int bt;
	int currentProcessedBT = 0;;
	string at;
	int core;

public:
	bool processDone = false;
	void NewProcess(int newId, int newBT, string timeArrived);
	void IncreaseProcessBT();
	void CreateNewFile();
	int GetPID();
	string GetAT();
	int GetCurrentProgress();
	int GetBT();
	int GetCoreValue();
	void SetCoreValue(int value);
};

void Process::NewProcess(int newId, int newBT, string timeArrived) {
	id = newId;
	bt = newBT;
	at = timeArrived;
}

void Process::IncreaseProcessBT() {
	if (currentProcessedBT < bt) {
		currentProcessedBT++;
	}
}

void Process::CreateNewFile() {
	// Get Desktop path
	char desktopPath[MAX_PATH];
	SHGetFolderPathA(NULL, CSIDL_DESKTOP, NULL, 0, desktopPath);

	// Build folder and file path
	string folderPath = string(desktopPath) + "\\TextFiles";
	CreateDirectoryA(folderPath.c_str(), NULL);
	string filePath = folderPath + "\\process" + to_string(id) + ".txt";

	ofstream file(filePath);
	for (int i = 0; i < bt; i++) {
		file << "Hello World from Core " << core << endl;
	}
	file.close();
}

int Process::GetPID() {
	return id;
}

string Process::GetAT() {
	return at;
}

int Process::GetCurrentProgress() {
	return currentProcessedBT;
}

int Process::GetBT() {
	return bt;
}

int Process::GetCoreValue() {
	return core;
}

void Process::SetCoreValue(int value) {
	core = value;
}

// 14/06/2025

Session sessions[10];

void initialize() {
	printColor("\"initialize\" command recognized. Doing something...\n", YELLOW);
	// TODO: Implement the initialize command
}

void screen(int currentSession) {
	//printColor("\"screen\" command recognized. Doing something...\n", YELLOW);
	// TODO: Implement the screen command

	//Display session name and time created
	printColor(sessions[currentSession].name + "\n\n", YELLOW);
	printColor(sessions[currentSession].timestamp + "\n", YELLOW);
	cout << "Total commands happened on screen: " << sessions[currentSession].totalLines << "\n\n";
	cout << "Previous command done: " << sessions[currentSession].currentLine << "\n\n";

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
			sessions[currentSession].totalLines++;
			sessions[currentSession].currentLine = command;
		}
	}
	system("cls");

	printBanner();
	printSubtitle();
}

void schedulerTest() {
	printColor("\"scheduler-test\" command recognized. Doing something...\n", YELLOW);

	srand(time(0));
	queue<Process> readyQueue;
	queue<Process> doneQueue;
	int createProcess;
	int currentPidInc = 1;
	Process cores[4];
	int currentCoreFree = 0;
	Process emptyProcess;
	bool isAssigned = false;

	emptyProcess.NewProcess(0, 0, "");

	for (int i = 0; i < 4; i++) {
		cores[i] = emptyProcess;
	}

	while (true) {
		for (int i = 0; i < 4; i++) {
			if (cores[i].GetBT() > 0) {
				cores[i].IncreaseProcessBT();
				if (cores[i].GetCurrentProgress() == cores[i].GetBT()) {
					currentCoreFree = i;
					cores[i].processDone = true;
					cores[i].CreateNewFile();
					if (readyQueue.empty() == 0) {
						doneQueue.push(cores[i]);
						readyQueue.front().SetCoreValue(currentCoreFree);
						cores[i] = readyQueue.front();
						readyQueue.pop();
					}
					else {
						doneQueue.push(cores[i]);
						cores[i] = emptyProcess;
					}
				}
			}
		}

		createProcess = rand() % 100;
		isAssigned = false;
		if (currentPidInc <= 10) {
			if (createProcess < 49) {
				Process newProcess;

				newProcess.NewProcess(currentPidInc, rand() % 10 + 1, "Insert arrival time");
				currentPidInc++;

				for (int i = 0; i < 4; i++) {
					if (cores[i].GetBT() == 0) {
						cores[i] = newProcess;
						cores[i].SetCoreValue(i);
						isAssigned = true;
						break;
					}
				}

				if (isAssigned == false) {
					readyQueue.push(newProcess);
				}
			}
		}

		system("cls");

		std::cout << "--------------------------------" << endl;
		std::cout << "Running processes: " << endl;
		for (int i = 0; i < 4; i++) {
			if (cores[i].GetBT() > 0) {
				std::cout << "process" << cores[i].GetPID() << "    " << cores[i].GetAT() << "    " << "Core: " << cores[i].GetCoreValue() << "    " << cores[i].GetCurrentProgress() << "/" << cores[i].GetBT() << endl;
			}
		}

		std::cout << endl;

		std::cout << "Finished processes: " << endl;
		for (int i = 0; i < doneQueue.size(); i++) {
			std::cout << "process" << doneQueue.front().GetPID() << "    " << doneQueue.front().GetAT() << "    " << "Core: " << doneQueue.front().GetCoreValue() << "    " << doneQueue.front().GetCurrentProgress() << "/" << doneQueue.front().GetBT() << endl;
			doneQueue.push(doneQueue.front());
			doneQueue.pop();
		}

		std::cout << "--------------------------------" << endl;

		Sleep(850);
	}
}

void schedulerStop() {
	printColor("\"scheduler-stop\" command recognized. Doing something...\n", YELLOW);
	// TODO: Implement the scheduler-stop command
}

void reportUtil() {
	printColor("\"report-util\" command recognized. Doing something...\n", YELLOW);
	// TODO: Implement the report-util command
}

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
	//Holds the current index for new sessions
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
					for (Session session : sessions) {
						if (session.name == command.substr(command.find("-s") + 3)) {
							screenFound = true;
							break;
						}
					}

					if (!screenFound) {
						//If session name does not exist, create new session
						sessions[currentSessionCount] = { command.substr(command.find("-s") + 3), "", 0, getCurrentTimestamp() };
						system("cls");
						screen(currentSessionCount);
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
						if (session.name == command.substr(command.find("-r") + 3)) {
							screenFound = true;
							break;
						}
						sessionToResume++;
					}

					if (screenFound) {
						//If session name exists, resume session
						system("cls");
						screen(sessionToResume);
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

		}
		break;
	}
}
