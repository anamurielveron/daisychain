#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <queue>
#include <fstream>
#include <iomanip>
#include <sstream>

using namespace std;

#include "FCFSScheduler.h"
#include "utils.h"

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
    }
    this_thread::sleep_for(chrono::milliseconds(100));
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