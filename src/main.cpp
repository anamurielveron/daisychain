// daisychain.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <iomanip>
#include <sstream>
#include <windows.h>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <vector>
#include <algorithm>

#include "utils.h"

using namespace std;

// Forward declarations
class Process;
class FCFSScheduler;

// Global scheduler instance
FCFSScheduler* globalScheduler = nullptr;

/**
* PROCESS CLASS
*/
class Process {
private:
    int id;
    int bt;
    int currentProcessedBT = 0;
    string at;
    int core = -1;
    vector<string> printLogs;

public:
    bool processDone = false;

    void NewProcess(int newId, int newBT, string timeArrived);
    void IncreaseProcessBT();
    void CreateNewFile();
    void AddPrintLog(const string& message, int coreNum);
    int GetPID() const;
    string GetAT() const;
    int GetCurrentProgress() const;
    int GetBT() const;
    int GetCoreValue() const;
    void SetCoreValue(int value);
    vector<string> GetPrintLogs() const;
};

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

/**
* FCFS SCHEDULER CLASS
*/
class FCFSScheduler {
private:
    static const int NUM_CORES = 4;
    Process cores[NUM_CORES];
    queue<Process> readyQueue;
    queue<Process> doneQueue;
    mutex schedulerMutex;
    atomic<bool> running;
    atomic<int> currentPidInc;
    Process emptyProcess;
    thread schedulerThread;

public:
    FCFSScheduler();
    ~FCFSScheduler();
    void Start();
    void Stop();
    void SchedulerLoop();
    void CreateProcess();
    void DisplayStatus();
    bool IsRunning();
    queue<Process> GetFinishedProcesses();
};

FCFSScheduler::FCFSScheduler() : running(false), currentPidInc(1) {
    emptyProcess.NewProcess(0, 0, "");
    for (int i = 0; i < NUM_CORES; i++) {
        cores[i] = emptyProcess;
    }
}

FCFSScheduler::~FCFSScheduler() {
    Stop();
}

void FCFSScheduler::Start() {
    if (!running) {
        running = true;
        schedulerThread = thread(&FCFSScheduler::SchedulerLoop, this);
    }
}

void FCFSScheduler::Stop() {
    if (running) {
        running = false;
        if (schedulerThread.joinable()) {
            schedulerThread.join();
        }
    }
}

void FCFSScheduler::SchedulerLoop() {
    srand(time(0));

    while (running) {
        lock_guard<mutex> lock(schedulerMutex);

        // Process cores
        for (int i = 0; i < NUM_CORES; i++) {
            if (cores[i].GetBT() > 0) {
                cores[i].IncreaseProcessBT();

                // Add print command simulation - every execution adds a print command
                string message = "\"Hello world from screen_" +
                    string(2 - to_string(cores[i].GetPID()).length(), '0') +
                    to_string(cores[i].GetPID()) + "!\"";
                cores[i].AddPrintLog(message, i);

                // Check if process is done
                if (cores[i].GetCurrentProgress() == cores[i].GetBT()) {
                    cores[i].processDone = true;
                    cores[i].CreateNewFile();
                    doneQueue.push(cores[i]);

                    // Assign next process from ready queue if available
                    if (!readyQueue.empty()) {
                        readyQueue.front().SetCoreValue(i);
                        cores[i] = readyQueue.front();
                        readyQueue.pop();
                    }
                    else {
                        cores[i] = emptyProcess;
                    }
                }
            }
        }

        // Create new process randomly (only if we haven't reached the limit)
        if (currentPidInc <= 10) {
            int createProcess = rand() % 100;
            if (createProcess < 20) { // 20% chance to create process
                Process newProcess;
                newProcess.NewProcess(currentPidInc, 100, getCurrentTimestamp()); // 100 print commands
                currentPidInc++;

                bool assigned = false;
                for (int i = 0; i < NUM_CORES; i++) {
                    if (cores[i].GetBT() == 0) {
                        cores[i] = newProcess;
                        cores[i].SetCoreValue(i);
                        assigned = true;
                        break;
                    }
                }

                if (!assigned) {
                    readyQueue.push(newProcess);
                }
            }
        }

        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

void FCFSScheduler::CreateProcess() {
    lock_guard<mutex> lock(schedulerMutex);

    if (currentPidInc <= 10) {
        Process newProcess;
        newProcess.NewProcess(currentPidInc, 100, getCurrentTimestamp());
        currentPidInc++;

        bool assigned = false;
        for (int i = 0; i < NUM_CORES; i++) {
            if (cores[i].GetBT() == 0) {
                cores[i] = newProcess;
                cores[i].SetCoreValue(i);
                assigned = true;
                break;
            }
        }

        if (!assigned) {
            readyQueue.push(newProcess);
        }

        //printColor("Process " + to_string(currentPidInc - 1) + " created.\n", GREEN);
    }
    else {
        /*printColor("Maximum number of processes (10) reached.\n", YELLOW);*/
    }
}

void FCFSScheduler::DisplayStatus() {
    lock_guard<mutex> lock(schedulerMutex);

    system("cls");
    printBanner();
    printSubtitle();

    // Count active cores
    int activeCores = 0;
    for (int i = 0; i < NUM_CORES; i++) {
        if (cores[i].GetBT() > 0) {
            activeCores++;
        }
    }

    cout << "CPU Utilization: " << ((float)activeCores / NUM_CORES) * 100 << "%" << endl;
    cout << "Cores used: " << activeCores << " / " << NUM_CORES << endl;
    cout << "Cores available: " << (NUM_CORES - activeCores) << " / " << NUM_CORES << endl;

    cout << "\n--------------------------------" << endl;
    cout << "Running processes:" << endl;
    for (int i = 0; i < NUM_CORES; i++) {
        if (cores[i].GetBT() > 0) {
            cout << "screen_" << setfill('0') << setw(2) << cores[i].GetPID() << "    "
                << cores[i].GetAT() << "    Core: " << cores[i].GetCoreValue()
                << "    " << cores[i].GetCurrentProgress() << "/" << cores[i].GetBT() << endl;
        }
    }

    cout << "\nFinished processes:" << endl;
    queue<Process> tempQueue = doneQueue;
    while (!tempQueue.empty()) {
        Process p = tempQueue.front();
        cout << "screen_" << setfill('0') << setw(2) << p.GetPID() << "    " << p.GetAT()
            << "    Finished    " << p.GetCurrentProgress() << "/" << p.GetBT() << endl;
        tempQueue.pop();
    }
    cout << "--------------------------------" << endl;
}

bool FCFSScheduler::IsRunning() {
    return running;
}

queue<Process> FCFSScheduler::GetFinishedProcesses() {
    return doneQueue;
}

/**
* SESSION CLASS
*/
class Session {
    string name;
    string currentLine;
    int totalLines = 0;
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

void Session::screen() {
    printColor(name + "\n\n", YELLOW);
    printColor(timestamp + "\n", YELLOW);
    cout << "Total commands happened on screen: " << totalLines << "\n\n";
    cout << "Previous command done: " << currentLine << "\n\n";

    printPlaceHolderConsoles();
    while (true) {
        string command;
        printColor("~> ", GREEN);
        getline(cin, command);

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
            printColor("Some Strcmp function to search? \n", YELLOW);
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

/**
* COMMAND FUNCTIONS
*/
void initialize() {
    printColor("\"initialize\" command recognized. Initializing scheduler...\n", YELLOW);
    if (globalScheduler == nullptr) {
        globalScheduler = new FCFSScheduler();
        globalScheduler->Start();
        printColor("Scheduler initialized with 4 cores.\n", GREEN);
    }
    else {
        printColor("Scheduler already initialized.\n", YELLOW);
    }
}

void schedulerTest() {
    printColor("\"scheduler-test\" command recognized. Starting test...\n", YELLOW);
    if (globalScheduler != nullptr && globalScheduler->IsRunning()) {
        // Create 10 processes for testing
        for (int i = 0; i < 10; i++) {
            globalScheduler->CreateProcess();
            this_thread::sleep_for(chrono::milliseconds(200));
        }
        printColor("Created 10 test processes.\n", GREEN);
    }
    else {
        printColor("Scheduler not initialized. Please run 'initialize' first.\n", RED);
    }
}

void schedulerStop() {
    printColor("\"scheduler-stop\" command recognized. Stopping scheduler...\n", YELLOW);
    if (globalScheduler != nullptr) {
        globalScheduler->Stop();
        delete globalScheduler;
        globalScheduler = nullptr;
        printColor("Scheduler stopped.\n", GREEN);
    }
    else {
        printColor("Scheduler not running.\n", YELLOW);
    }
}

void reportUtil() {
    printColor("\"report-util\" command recognized. Generating report...\n", YELLOW);
    if (globalScheduler != nullptr) {
        globalScheduler->DisplayStatus();
    }
    else {
        printColor("Scheduler not initialized. Please run 'initialize' first.\n", RED);
    }
}

/**
* MAIN FUNCTION
*/
int main() {
    int currentSessionCount = 0;
    int sessionToResume = 0;
    bool screenFound = false;

    while (true) {
        printBanner();
        printSubtitle();

        while (true) {
            screenFound = false;
            string command;
            printColor("~> ", GREEN);
            getline(cin, command);

            if (command == "help") {
                printHelp();
            }
            else if (command == "initialize") {
                initialize();
            }
            else if (command.find("screen") != string::npos) {
                if (command.find("-s") != string::npos) {
                    if (command.substr(command.find("-s") + 2) != "" && command.substr(command.find("-s") + 3) != "") {
                        for (Session session : sessions) {
                            if (session.GetName() == command.substr(command.find("-s") + 3)) {
                                screenFound = true;
                                break;
                            }
                        }

                        if (!screenFound) {
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

                        for (Session session : sessions) {
                            if (session.GetName() == command.substr(command.find("-r") + 3)) {
                                screenFound = true;
                                break;
                            }
                            sessionToResume++;
                        }

                        if (screenFound) {
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
                else if (command.find("-ls") != string::npos) {
                    printColor("Active screens:\n", YELLOW);
                    bool hasScreens = false;
                    for (int i = 0; i < currentSessionCount; i++) {
                        if (sessions[i].GetName() != "") {
                            cout << "- " << sessions[i].GetName() << endl;
                            hasScreens = true;
                        }
                    }
                    if (!hasScreens) {
                        printColor("No active screens.\n", CYAN);
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
                if (globalScheduler != nullptr) {
                    schedulerStop();
                }
                break;
            }
            else {
                printColor("Unknown command. Type 'help' for a list of commands.\n", RED);
            }
        }
        break;
    }

    return 0;
}