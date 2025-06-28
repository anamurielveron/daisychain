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
* RR SCHEDULER CLASS
*/
class RRScheduler {
private:
    static const int NUM_CORES = 4;
    static const int QUANTUM = 10;

    struct CoreSlot {
        Process proc;
        int qRemaining;
        bool isEmpty;

        CoreSlot() : qRemaining(0), isEmpty(true) {}
    };

    CoreSlot cores[NUM_CORES];
    queue<Process> readyQueue;
    queue<Process> doneQueue;
    mutex schedulerMutex;
    atomic<bool> running;
    atomic<int> currentPidInc;
    Process emptyProcess;
    thread schedulerThread;

public:
    RRScheduler();
    ~RRScheduler();
    void Start();
    void Stop();
    void SchedulerLoop();
    void CreateProcess();
    void DisplayStatus();
    bool IsRunning();
    queue<Process> GetFinishedProcesses();
};

RRScheduler::RRScheduler() : running(false), currentPidInc(1) {
    emptyProcess.NewProcess(0, 0, "");
    for (int i = 0; i < NUM_CORES; i++) {
        cores[i].proc = emptyProcess;
        cores[i].qRemaining = 0;
        cores[i].isEmpty = true;
    }
}

RRScheduler::~RRScheduler() {
    Stop();
}

void RRScheduler::Start() {
    if (!running) {
        running = true;
        schedulerThread = thread(&RRScheduler::SchedulerLoop, this);
    }
}

void RRScheduler::Stop() {
    if (running) {
        running = false;
        if (schedulerThread.joinable()) {
            schedulerThread.join();
        }
    }
}

void RRScheduler::SchedulerLoop() {
    srand(time(0));

    while (running) {
        {
            lock_guard<mutex> lock(schedulerMutex);

            // Process cores
            for (int i = 0; i < NUM_CORES; i++) {
                if (!cores[i].isEmpty && cores[i].proc.GetBT() > 0) {
                    // Execute one time unit of the process
                    cores[i].proc.IncreaseProcessBT();
                    cores[i].qRemaining--;

                    // Add print command simulation
                    string message = "\"Hello world from screen_" +
                        string(2 - to_string(cores[i].proc.GetPID()).length(), '0') +
                        to_string(cores[i].proc.GetPID()) + "!\"";
                    cores[i].proc.AddPrintLog(message, i);

                    // Check if process is done
                    if (cores[i].proc.GetCurrentProgress() == cores[i].proc.GetBT()) {
                        cores[i].proc.processDone = true;
                        cores[i].proc.CreateNewFile();
                        doneQueue.push(cores[i].proc);

                        // Assign next process from ready queue if available
                        if (!readyQueue.empty()) {
                            cores[i].proc = readyQueue.front();
                            cores[i].proc.SetCoreValue(i);
                            cores[i].qRemaining = QUANTUM;
                            cores[i].isEmpty = false;
                            readyQueue.pop();
                        }
                        else {
                            cores[i].proc = emptyProcess;
                            cores[i].qRemaining = 0;
                            cores[i].isEmpty = true;
                        }
                    }
                    // Check if quantum is exhausted but process is not done
                    else if (cores[i].qRemaining <= 0) {
                        // Move process back to ready queue (preemption)
                        readyQueue.push(cores[i].proc);

                        // Assign next process from ready queue if available
                        if (!readyQueue.empty()) {
                            cores[i].proc = readyQueue.front();
                            cores[i].proc.SetCoreValue(i);
                            cores[i].qRemaining = QUANTUM;
                            cores[i].isEmpty = false;
                            readyQueue.pop();
                        }
                        else {
                            cores[i].proc = emptyProcess;
                            cores[i].qRemaining = 0;
                            cores[i].isEmpty = true;
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
                        if (cores[i].isEmpty) {
                            cores[i].proc = newProcess;
                            cores[i].proc.SetCoreValue(i);
                            cores[i].qRemaining = QUANTUM;
                            cores[i].isEmpty = false;
                            assigned = true;
                            break;
                        }
                    }

                    if (!assigned) {
                        readyQueue.push(newProcess);
                    }
                }
            }
        }

        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

void RRScheduler::CreateProcess() {
    lock_guard<mutex> lock(schedulerMutex);

    if (currentPidInc <= 10) {
        Process newProcess;
        newProcess.NewProcess(currentPidInc, 100, getCurrentTimestamp());
        currentPidInc++;

        bool assigned = false;
        for (int i = 0; i < NUM_CORES; i++) {
            if (cores[i].isEmpty) {
                cores[i].proc = newProcess;
                cores[i].proc.SetCoreValue(i);
                cores[i].qRemaining = QUANTUM;
                cores[i].isEmpty = false;
                assigned = true;
                break;
            }
        }

        if (!assigned) {
            readyQueue.push(newProcess);
        }
    }
}

void RRScheduler::DisplayStatus() {
    lock_guard<mutex> lock(schedulerMutex);

    system("cls");
    printBanner();
    printSubtitle();

    // Count active cores
    int activeCores = 0;
    for (int i = 0; i < NUM_CORES; i++) {
        if (!cores[i].isEmpty && cores[i].proc.GetBT() > 0) {
            activeCores++;
        }
    }

    cout << "CPU Utilization: " << ((float)activeCores / NUM_CORES) * 100 << "%" << endl;
    cout << "Cores used: " << activeCores << " / " << NUM_CORES << endl;
    cout << "Cores available: " << (NUM_CORES - activeCores) << " / " << NUM_CORES << endl;
    cout << "Scheduling Algorithm: Round Robin (Quantum = " << QUANTUM << ")" << endl;

    cout << "\n--------------------------------" << endl;
    cout << "Running processes:" << endl;
    for (int i = 0; i < NUM_CORES; i++) {
        if (!cores[i].isEmpty && cores[i].proc.GetBT() > 0) {
            cout << "screen_" << setfill('0') << setw(2) << cores[i].proc.GetPID() << "    "
                << cores[i].proc.GetAT() << "    Core: " << cores[i].proc.GetCoreValue()
                << "    " << cores[i].proc.GetCurrentProgress() << "/" << cores[i].proc.GetBT()
                << "    Quantum remaining: " << cores[i].qRemaining << endl;
        }
    }

    cout << "\nReady Queue Size: " << readyQueue.size() << endl;

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

bool RRScheduler::IsRunning() {
    return running;
}

queue<Process> RRScheduler::GetFinishedProcesses() {
    return doneQueue;
}

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
