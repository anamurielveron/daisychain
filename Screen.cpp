#include<iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <list>
#include <windows.h>
#include "Screen.h"

using namespace std;

void Screen::screen() {
	//Display session name and time created
	printColor("Welcome to " + name + "\n\n", YELLOW);
	printColor("Time created: " + arrivalTimestamp + "\n", YELLOW);

	printPlaceHolderConsoles();
	while (true) {
		std::string command;
		printColor("~> ", CYAN);
		std::getline(std::cin, command);
		if (command == "process-smi") {
			printColor("Screen name: " + name + "\n", CYAN);
			printColor("ID: " + to_string(id) + "\n", CYAN);
			printColor("Current instruction line: " + to_string(executedInstructions) + "/" + to_string(totalInstructions) + "\n", CYAN);
			PrintProcessRunning();
		}
		else if (command == "exit") {
			break;
		}
	}
	system("cls");

	printBanner();
	printSubtitle();
}

void Screen::run() {
	while (executedInstructions < totalInstructions) {
		RunInstructions(rand() % 6);
	}
}

void Screen::RunInstructions(int instructionToRun) {
	if (instructionToRun == 0) {
		PRINT(printMsg);
	}
	else if (instructionToRun == 1) {
		int varNum = rand() % 101;
		string variable = "var" + to_string(varNum);
		DECLARE(variable, rand() % 500);
	}
	else if (instructionToRun == 2) {
		int varNum[3];
		string variable[3];
		for (int i = 0; i < 3; i++) {
			varNum[i] = rand() % 101;
			if(rand())
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

void Screen::PrintProcessRunning() {
	printColor("Log: \n", CYAN);
	for (string print : printLogs) {
		printColor(print + "\n", INVERTED);
	}
}

bool Screen::CheckVariable(string input) {
	for (char c : input) {
		if (!isdigit(c)) {
			return false;
		}
	}

	return true;
}

int Screen::FindVariable(string varInput) {
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

int Screen::ValueAssignment(string variable) {
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

void Screen::PRINT(string msg) {
	printLogs.push_back(getCurrentTimestamp() + " Core: " + to_string(coreAssigned) + " " + msg);
}

void Screen::DECLARE(string name, int val) {
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

	//printedLines.push_back(getCurrentTimestamp() + " New variable added: " + name);
}

void Screen::ADD(string sum, string addend1, string addend2) {
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

	//printedLines.push_back(getCurrentTimestamp() + " Added: " + addend1 + " + " + addend2 + " = " + sum);
}

void Screen::SUB(string diff, string subend1, string subend2) {
	int varDiffIndex = FindVariable(diff);
	int varSubEnd1 = ValueAssignment(subend1);
	int varSubEnd2 = ValueAssignment(subend2);
	int tempDiff = varSubEnd1 - varSubEnd2;

	if (varDiffIndex == -1) {
		DECLARE(diff, tempDiff);
	}
	else {
		vars[varDiffIndex].value = tempDiff;
	}

	//printedLines.push_back(getCurrentTimestamp() + " Subtracted: " + subend1 + " + " + subend2 + " = " + diff);
}

void Screen::FOR(int iterations) {
	nestedLoopNum++;
	
	list<int> instructionToRun;

	if (rand() % 2 == 0) {
		instructionToRun.push_back(rand() % 6);
	}
	else {
		int i = 0;
		int arrLen = (rand() % 10) + 2;
		while(i < arrLen) {
			instructionToRun.push_back(rand() % 6);
			i++;
		}
	}

	for (int i = 0; i < iterations; i++) {
		for(int j: instructionToRun) {
			RunInstructions(j);
		}
	}

	printLogs.push_back(getCurrentTimestamp() + " Core: " + to_string(coreAssigned) + " Looping finished!");
}

void Screen::SLEEP(int cycles) {
	Sleep(cycles);
}


void Screen::ExecuteInstruction(int coreNum) {
	if (executedInstructions < totalInstructions) {
		executedInstructions++;
		RunInstructions(rand() % 6);
		// Simulate a PRINT instruction
		//string message = "\"Hello world from " + name + "! (Inst: " + to_string(executedInstructions) + ")\"";
		//AddPrintLog(message, coreNum);
		if (executedInstructions == totalInstructions) {
			finished.store(true); // Use store for atomic boolean
		}
	}
}

Screen::Screen(std::string pid, int processSize)
	: pid(pid), processSize(processSize) {
}

std::string Screen::getPID() const {
	return pid;
}

int Screen::getProcessSize() const {
	return processSize;
}

void Screen::addSymbol(std::string symbol, int address) {
	symbolTable[symbol] = address;
}

int Screen::getSymbolAddress(const std::string& symbol) const {
	auto it = symbolTable.find(symbol);
	if (it != symbolTable.end()) return it->second;
	return -1;
}

bool Screen::hasSymbol(const std::string& symbol) const {
	return symbolTable.find(symbol) != symbolTable.end();
}

const std::unordered_map<std::string, int>& Screen::getSymbolTable() const {
	return symbolTable;
}


/*
void Screen::AddPrintLog(const string& message, int coreNum) {
    SYSTEMTIME st;
    GetLocalTime(&st);

    string am_pm = (st.wHour >= 12) ? "PM" : "AM";
    int hour = st.wHour % 12;
    if (hour == 0) hour = 12;

    char timestamp[100];
    sprintf_s(timestamp, "(%02d/%02d/%04d %02d:%02d:%02d%s)",
        st.wMonth, st.wDay, st.wYear,
        hour, st.wMinute, st.wSecond, am_pm.c_str());

    string logEntry = string(timestamp) + " Core: " + to_string(coreNum) + " " + message;
    printLogs.push_back(logEntry);
}
*/

void Screen::setProcessName(const std::string& name) {
	process_name = name;
}

std::string Screen::getProcessName() const {
	return process_name;
}

void Screen::setMemoryAddress(int address) {
	memory_address = address;
}

int Screen::getMemoryAddress() const {
	return memory_address;
}

void Screen::setMemoryRequirement(int mem) {
	memoryRequirement = mem;
}

int Screen::getMemoryRequirement() const {
	return memoryRequirement;
}

void Screen::addToSymbolTable(const std::string& var, int memAddr) {
	symbolTable[var] = memAddr;
}

void Screen::clearSymbolTable() {
	symbolTable.clear();
}

void Screen::printSymbolTable() const {
	std::cout << "[Symbol Table for " << process_name << "]\n";
	for (const auto& entry : symbolTable) {
		std::cout << "  " << entry.first << " = " << entry.second << "\n";
	}
}



int Screen::GetPID() const { return id; }
string Screen::GetName() const { return name; }
string Screen::GetArrivalTime() const { return arrivalTimestamp; }
unsigned int Screen::GetExecutedInstructions() const { return executedInstructions; }
unsigned int Screen::GetTotalInstructions() const { return totalInstructions; }
int Screen::GetCoreValue() const { return coreAssigned; }
void Screen::SetCoreValue(int value) { coreAssigned = value; }
vector<string> Screen::GetPrintLogs() const { return printLogs; }
bool Screen::IsFinished() const { return finished.load(); } // Use load for atomic boolean
void Screen::SetFinished(bool status) { finished.store(status); } // Use store for atomic boolean