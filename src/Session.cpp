#include "Session.h"
#include "utils.h"
#include <windows.h>

void Session::newSession(string scrName, string timeCreated) {
	name = scrName;
	currentLine = "";
	timestamp = timeCreated;

	printMsg = "Hello from " + scrName + "!";
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

void Session::RunInstructions(int instructionToRun) {
	if (instructionToRun == 0) {
		PRINT(printMsg);
	}
	else if (instructionToRun == 1){
		int varNum = rand() % 101;
		string variable = "var" + to_string(varNum);
		DECLARE(variable, rand() % 500);
	}
	else if (instructionToRun == 2) {
		int varNum[3];
		string variable[3];
		for (int i = 0; i < 3; i++){
			varNum[i] = rand() % 101;
			variable[i] = "var" + to_string(varNum[i]);
		}
		
		ADD(variable[0], variable[1], variable[2]);
	}
	else if (instructionToRun == 3) {
		int varNum[3];
		string variable[3];
		for (int i = 0; i < 3; i++) {
			varNum[i] = rand() % 101;
			variable[i] = "var" + to_string(varNum[i]);
		}

		SUB(variable[0], variable[1], variable[2]);
	}
	else if (instructionToRun == 4) {
		if (nestedLoopNum == 3) {
			RunInstructions(rand() % 6);
		}
	}
	else {
		SLEEP(10);
	}
}

bool Session::CheckVariable(string input) {
	for (char c : input) {
		if (!isdigit(c)) {
			return false;
		}
	}

	return true;
}

int Session::FindVariable(string varInput) {
	for (int i = 0; i < 100;i++) {
		if (vars[i].varName == varInput) {
			return i;
		}
		else if (vars[i].varName == "empty") {
			return -1;
		}
	}

	return -1;
}

int Session::ValueAssignment(string variable) {
	int foundIndex;
	int constantVar;

	if (CheckVariable(variable)) {
		constantVar = stoi(variable);
	}
	else
	{
		foundIndex = FindVariable(variable);
		if (foundIndex == -1) {
			DECLARE(variable, 0);
			constantVar = 0;
		}
		else {
			constantVar = vars[foundIndex].value;
		}
	}

	return constantVar;
}

void Session::PRINT(string msg) {
	printedLines.push_back(getCurrentTimestamp() + " " + msg);
}

void Session::DECLARE(string name, int val) {
	bool exists = false;
	int foundIndex = FindVariable(name);

	if (foundIndex == -1) {
		for (int i = 0; i < 100; i++) {
			if (vars[i].varName == "empty") {
				vars[i].varName = name;
				vars[i].value = val;
				break;
			}
		}
	}
	else {
		vars[foundIndex].value = val;
	}
}

void Session::ADD(string sum, string addend1, string addend2) {
	int varSumIndex = FindVariable(sum);
	int varAddEnd1 = ValueAssignment(addend1);
	int varAddEnd2 = ValueAssignment(addend2);
	int tempSum = varAddEnd1 + varAddEnd2;

	if (varSumIndex == -1) {
		DECLARE(sum, tempSum);
	}
	else {
		vars[varSumIndex].value = tempSum;
	}
}

void Session::SUB(string diff, string subend1, string subend2) {
	int varDiffIndex = FindVariable(diff);
	int varSubEnd1 = ValueAssignment(subend1);
	int varSubEnd2 = ValueAssignment(subend2);
	int tempDiff = varSubEnd1 + varSubEnd2;

	if (varDiffIndex == -1) {
		DECLARE(diff, tempDiff);
	}
	else {
		vars[varDiffIndex].value = tempDiff;
	}
}

void Session::FOR(int iterations) {
	nestedLoopNum++;

	for (int i = 0; i < iterations; i++) {
		RunInstructions(rand() % 6);
	}
}

void Session::SLEEP(int cycles) {
	Sleep(cycles);
}