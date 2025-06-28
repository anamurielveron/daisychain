#include<iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <windows.h>
#include "Process.h"

using namespace std;

void Process::ExecuteInstruction(int coreNum) {
    if (executedInstructions < totalInstructions) {
        executedInstructions++;
        // Simulate a PRINT instruction
        string message = "\"Hello world from " + name + "! (Inst: " + to_string(executedInstructions) + ")\"";
        AddPrintLog(message, coreNum);
    }
    if (executedInstructions == totalInstructions) {
        finished.store(true); // Use store for atomic boolean
    }
}

void Process::AddPrintLog(const string& message, int coreNum) {
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

int Process::GetPID() const { return id; }
string Process::GetName() const { return name; }
string Process::GetArrivalTime() const { return arrivalTimestamp; }
unsigned int Process::GetExecutedInstructions() const { return executedInstructions; }
unsigned int Process::GetTotalInstructions() const { return totalInstructions; }
int Process::GetCoreValue() const { return coreAssigned; }
void Process::SetCoreValue(int value) { coreAssigned = value; }
vector<string> Process::GetPrintLogs() const { return printLogs; }
bool Process::IsFinished() const { return finished.load(); } // Use load for atomic boolean
void Process::SetFinished(bool status) { finished.store(status); } // Use store for atomic boolean