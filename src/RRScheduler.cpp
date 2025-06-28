#include "RRScheduler.h"


RRScheduler::RRScheduler(int num_cores, int quantum_cycles, unsigned int min_ins, unsigned int max_ins, int batch_freq, int delay_exec)
    : running(false), currentPidInc(1), cpuCycles(0), numCores(num_cores), quantum(quantum_cycles),
    minInstructions(min_ins), maxInstructions(max_ins), batchProcessFrequency(batch_freq),
    delayPerExecution(delay_exec) {
    cores.resize(numCores);
    for (int i = 0; i < numCores; i++) {
        cores[i].proc = nullptr; // Initialize unique_ptr to nullptr
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
        std::thread schedulerThread(&RRScheduler::SchedulerLoop, this);
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
    srand(static_cast<unsigned int>(time(0)));

    while (running) {
        {
            lock_guard<mutex> lock(schedulerMutex);

            cpuCycles++; // Increment CPU cycle

            // Screen cores
            for (int i = 0; i < numCores; i++) {
                if (!cores[i].isEmpty && cores[i].proc && !cores[i].proc->IsFinished() && cores[i].proc->GetTotalInstructions() > 0) {
                    cores[i].proc->ExecuteInstruction(i);
                    cores[i].qRemaining--;

                    if (delayPerExecution > 0) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(delayPerExecution)); // Simulate busy-waiting
                    }

                    // Check if process is done
                    if (cores[i].proc->IsFinished()) {
                        doneQueue.push(std::move(cores[i].proc)); // Move unique_ptr
                        // Assign next process from ready queue if available
                        if (!readyQueue.empty()) {
                            unique_ptr<Screen> nextProcess = std::move(readyQueue.front());
                            readyQueue.pop();
                            nextProcess->SetCoreValue(i);
                            cores[i].proc = std::move(nextProcess);
                            cores[i].qRemaining = quantum;
                            cores[i].isEmpty = false;
                        }
                        else {
                            cores[i].proc.reset(); // Core becomes idle
                            cores[i].qRemaining = 0;
                            cores[i].isEmpty = true;
                        }
                    }
                    // Check if quantum is exhausted but process is not done (preemption)
                    else if (cores[i].qRemaining <= 0) {
                        readyQueue.push(std::move(cores[i].proc)); // Move process back to ready queue
                        // Assign next process from ready queue if available
                        if (!readyQueue.empty()) {
                            unique_ptr<Screen> nextProcess = std::move(readyQueue.front());
                            readyQueue.pop();
                            nextProcess->SetCoreValue(i);
                            cores[i].proc = std::move(nextProcess);
                            cores[i].qRemaining = quantum;
                            cores[i].isEmpty = false;
                        }
                        else {
                            cores[i].proc.reset(); // Core becomes idle
                            cores[i].qRemaining = 0;
                            cores[i].isEmpty = true;
                        }
                    }
                }
            }

            // Generate new processes based on batchProcessFrequency
            if (batchProcessFrequency > 0 && cpuCycles % batchProcessFrequency == 0) {
                bool assignedToCore = false;
                for (int i = 0; i < numCores; ++i) {
                    if (cores[i].isEmpty) {
                        string processName = "screen_" + string(2 - to_string(currentPidInc).length(), '0') + to_string(currentPidInc);
                        unsigned int instructions = minInstructions + (rand() % (maxInstructions - minInstructions + 1));
                        unique_ptr<Screen> newProcess = std::make_unique<Screen>(currentPidInc, instructions, getCurrentTimestamp(), processName);
                        cores[i].proc = std::move(newProcess);
                        cores[i].proc->SetCoreValue(i);
                        cores[i].qRemaining = quantum;
                        cores[i].isEmpty = false;
                        currentPidInc++;
                        assignedToCore = true;
                        break;
                    }
                }
                if (!assignedToCore) {
                    string processName = "screen_" + string(2 - to_string(currentPidInc).length(), '0') + to_string(currentPidInc);
                    unsigned int instructions = minInstructions + (rand() % (maxInstructions - minInstructions + 1));
                    readyQueue.push(std::make_unique<Screen>(currentPidInc, instructions, getCurrentTimestamp(), processName));
                    currentPidInc++;
                }
            }

            // Distribute processes from ready queue to idle cores if any
            for (int i = 0; i < numCores; ++i) {
                if (cores[i].isEmpty && !readyQueue.empty()) {
                    unique_ptr<Screen> nextProcess = std::move(readyQueue.front());
                    readyQueue.pop();
                    nextProcess->SetCoreValue(i);
                    cores[i].proc = std::move(nextProcess);
                    cores[i].qRemaining = quantum;
                    cores[i].isEmpty = false;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void RRScheduler::CreateProcess(bool isBatch, const string& userProvidedName) {
    lock_guard<mutex> lock(schedulerMutex);

    string processName;
    if (isBatch) {
        processName = "screen_" + string(2 - to_string(currentPidInc).length(), '0') + to_string(currentPidInc);
    }
    else {
        processName = userProvidedName;
    }

    unsigned int instructions = minInstructions + (rand() % (maxInstructions - minInstructions + 1));
    unique_ptr<Screen> newProcess = std::make_unique<Screen>(currentPidInc, instructions, getCurrentTimestamp(), processName);
    currentPidInc++;

    bool assigned = false;
    for (int i = 0; i < numCores; i++) {
        if (cores[i].isEmpty) {
            cores[i].proc = std::move(newProcess);
            cores[i].proc->SetCoreValue(i);
            cores[i].qRemaining = quantum;
            cores[i].isEmpty = false;
            assigned = true;
            break;
        }
    }

    if (!assigned) {
        readyQueue.push(std::move(newProcess));
    }
}

void RRScheduler::DisplayStatus(ostream& os) {
    lock_guard<mutex> lock(schedulerMutex);

    int activeCores = 0;
    for (int i = 0; i < numCores; i++) {
        if (!cores[i].isEmpty && cores[i].proc && !cores[i].proc->IsFinished() && cores[i].proc->GetTotalInstructions() > 0) {
            activeCores++;
        }
    }

    os << "CPU Utilization: " << std::fixed << std::setprecision(2) << ((float)activeCores / numCores) * 100 << "%" << endl;
    os << "Cores used: " << activeCores << " / " << numCores << endl;
    os << "Cores available: " << (numCores - activeCores) << " / " << numCores << endl;
    os << "Scheduling Algorithm: Round Robin (Quantum = " << quantum << ")" << endl;

    os << "\n--------------------------------" << endl;
    os << "Running processes:" << endl;
    for (int i = 0; i < numCores; i++) {
        if (!cores[i].isEmpty && cores[i].proc && !cores[i].proc->IsFinished() && cores[i].proc->GetTotalInstructions() > 0) {
            os << cores[i].proc->GetName() << "    "
                << cores[i].proc->GetArrivalTime() << "    Core: " << cores[i].proc->GetCoreValue()
                << "    " << cores[i].proc->GetExecutedInstructions() << "/" << cores[i].proc->GetTotalInstructions()
                << "    Quantum remaining: " << cores[i].qRemaining << endl;
        }
    }

    os << "\nReady Queue Size: " << readyQueue.size() << endl;

    os << "\nFinished processes:" << endl;
    queue<unique_ptr<Screen>> tempDoneQueue;
    while (!doneQueue.empty()) {
        tempDoneQueue.push(std::move(doneQueue.front()));
        doneQueue.pop();
    }
    while (!tempDoneQueue.empty()) {
        const unique_ptr<Screen>& p = tempDoneQueue.front();
        os << p->GetName() << "    " << p->GetArrivalTime()
            << "    Finished    " << p->GetExecutedInstructions() << "/" << p->GetTotalInstructions() << endl;
        doneQueue.push(std::move(tempDoneQueue.front())); // Move back
        tempDoneQueue.pop();
    }
    os << "--------------------------------" << endl;
}

bool RRScheduler::IsRunning() {
    return running.load();
}

Screen* RRScheduler::GetProcessByName(const string& name) {
    lock_guard<mutex> lock(schedulerMutex);
    for (int i = 0; i < numCores; ++i) {
        if (!cores[i].isEmpty && cores[i].proc && cores[i].proc->GetName() == name && !cores[i].proc->IsFinished()) {
            return cores[i].proc.get(); // Return raw pointer
        }
    }
    return nullptr; // Not found in running cores
}