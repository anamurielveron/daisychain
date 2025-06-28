#include<iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <windows.h>
#include "Process.h"

using namespace std;

void Process::NewProcess(int newId, int newBT, string timeArrived) {
    id = newId;
    bt = newBT;
    at = timeArrived;
    currentProcessedBT = 0;
    processDone = false;
    printLogs.clear();
    core = -1;
}

void Process::IncreaseProcessBT() {
    if (currentProcessedBT < bt) {
        currentProcessedBT++;
    }
}

void Process::CreateNewFile() {
    if (printLogs.empty()) return;

    string filename = "process_" + to_string(id) + ".txt";
    ofstream file(filename);

    if (file.is_open()) {
        file << "Process name: screen_" << setfill('0') << setw(2) << id << endl;
        file << "Logs:" << endl;

        for (const string& log : printLogs) {
            file << log << endl;
        }

        file.close();
        //cout << "Created file: " << filename << endl;
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

    string logEntry = string(timestamp) + " Core:" + to_string(coreNum) + " " + message;
    printLogs.push_back(logEntry);
}

int Process::GetPID() const { return id; }
string Process::GetAT() const { return at; }
int Process::GetCurrentProgress() const { return currentProcessedBT; }
int Process::GetBT() const { return bt; }
int Process::GetCoreValue() const { return core; }
void Process::SetCoreValue(int value) { core = value; }
vector<string> Process::GetPrintLogs() const { return printLogs; }