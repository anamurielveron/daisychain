#include "RRScheduler.h"

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