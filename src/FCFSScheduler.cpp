using namespace std;

#include "FCFSScheduler.h"
#include "utils.h"

string padNumberFCFS(int number, int width) {
    string numStr = to_string(number);
    if (numStr.length() >= width)
        return numStr;
    return string(width - numStr.length(), '0') + numStr;
}

FCFSScheduler::FCFSScheduler(int num_cores, unsigned int min_ins, unsigned int max_ins, int batch_freq, int delay_exec)
    : running(false), currentPidInc(1), cpuCycles(0), numCores(num_cores),
    minInstructions(min_ins), maxInstructions(max_ins), batchProcessFrequency(batch_freq),
    delayPerExecution(delay_exec) {
    cores.resize(numCores); // Resize with default-constructed unique_ptrs (nullptr)
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
    while (running) {
        {
            lock_guard<mutex> lock(schedulerMutex);

            cpuCycles++; // Increment CPU cycle

            // Process cores
            for (int i = 0; i < numCores; i++) {
                if (cores[i] && !cores[i]->IsFinished() && cores[i]->GetTotalInstructions() > 0) {
                    cores[i]->ExecuteInstruction(i);

                    if (delayPerExecution > 0) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(delayPerExecution));
                    }

                    if (cores[i]->IsFinished()) {
                        doneQueue.push(std::move(cores[i]));
                        if (!readyQueue.empty()) {
                            unique_ptr<Screen> nextProcess = std::move(readyQueue.front());
                            readyQueue.pop();
                            nextProcess->SetCoreValue(i);
                            cores[i] = std::move(nextProcess);
                        }
                        else {
                            cores[i].reset();
                        }
                    }
                }
            }

            // Assign processes from ready queue to idle cores
            for (int i = 0; i < numCores; ++i) {
                if (!cores[i] && !readyQueue.empty()) {
                    unique_ptr<Screen> nextProcess = std::move(readyQueue.front());
                    readyQueue.pop();
                    nextProcess->SetCoreValue(i);
                    cores[i] = std::move(nextProcess);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void FCFSScheduler::CreateProcess(bool isBatch, const string& userProvidedName) {
    lock_guard<mutex> lock(schedulerMutex);

    string processName;
    if (isBatch) {
        processName = "screen_" + padNumberFCFS(currentPidInc, 2);
    }
    else {
        processName = userProvidedName;
    }

    unsigned int instructions = minInstructions + (rand() % (maxInstructions - minInstructions + 1));
    unique_ptr<Screen> newProcess = std::make_unique<Screen>(currentPidInc, instructions, getCurrentTimestamp(), processName);
    currentPidInc++;

    bool assigned = false;
    for (int i = 0; i < numCores; i++) {
        if (!cores[i]) {
            cores[i] = std::move(newProcess);
            cores[i]->SetCoreValue(i);
            assigned = true;
            break;
        }
    }

    if (!assigned) {
        readyQueue.push(std::move(newProcess));
    }
}

void FCFSScheduler::CreateProcessWithMemory(bool isBatch, const string& processName, int memorySize) {
    lock_guard<mutex> lock(schedulerMutex);

    unsigned int instructions = minInstructions + (rand() % (maxInstructions - minInstructions + 1));
    unique_ptr<Screen> newProcess = std::make_unique<Screen>(currentPidInc, instructions, getCurrentTimestamp(), processName, memorySize);
    currentPidInc++;

    bool assigned = false;
    for (int i = 0; i < numCores; i++) {
        if (!cores[i]) {
            cores[i] = std::move(newProcess);
            cores[i]->SetCoreValue(i);
            assigned = true;
            break;
        }
    }

    if (!assigned) {
        readyQueue.push(std::move(newProcess));
    }
}

void FCFSScheduler::CreateProcessWithInstructions(const string& processName, int memorySize, const string& instructions) {
    lock_guard<mutex> lock(schedulerMutex);

    unique_ptr<Screen> newProcess = std::make_unique<Screen>(currentPidInc, getCurrentTimestamp(), processName, memorySize, instructions);
    currentPidInc++;

    bool assigned = false;
    for (int i = 0; i < numCores; i++) {
        if (!cores[i]) {
            cores[i] = std::move(newProcess);
            cores[i]->SetCoreValue(i);
            assigned = true;
            break;
        }
    }

    if (!assigned) {
        readyQueue.push(std::move(newProcess));
    }
}

void FCFSScheduler::DisplayStatus(ostream& os) {
    lock_guard<mutex> lock(schedulerMutex);

    int activeCores = 0;
    for (int i = 0; i < numCores; i++) {
        if (cores[i] && !cores[i]->IsFinished()) {
            activeCores++;
        }
    }

    os << "CPU Utilization: " << std::fixed << std::setprecision(2) << ((float)activeCores / numCores) * 100 << "%" << endl;
    os << "Cores used: " << activeCores << " / " << numCores << endl;
    os << "Cores available: " << (numCores - activeCores) << " / " << numCores << endl;
    os << "Scheduling Algorithm: FCFS" << endl;

    os << "\n--------------------------------" << endl;
    os << "Running processes:" << endl;
    for (int i = 0; i < numCores; i++) {
        if (cores[i] && !cores[i]->IsFinished()) {
            os << cores[i]->GetName() << "    "
                << cores[i]->GetArrivalTime() << "    Core: " << cores[i]->GetCoreValue()
                << "    " << cores[i]->GetExecutedInstructions() << "/" << cores[i]->GetTotalInstructions() << endl;
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
        doneQueue.push(std::move(tempDoneQueue.front()));
        tempDoneQueue.pop();
    }
    os << "--------------------------------" << endl;
}

bool FCFSScheduler::IsRunning() {
    return running.load();
}

Screen* FCFSScheduler::GetProcessByName(const string& name) {
    lock_guard<mutex> lock(schedulerMutex);

    for (int i = 0; i < numCores; ++i) {
        if (cores[i] && cores[i]->GetName() == name && !cores[i]->IsFinished()) {
            return cores[i].get();
        }
    }

    queue<unique_ptr<Screen>> tempReadyQueue;
    Screen* foundProcess = nullptr;

    while (!readyQueue.empty()) {
        if (readyQueue.front()->GetName() == name && !readyQueue.front()->IsFinished()) {
            foundProcess = readyQueue.front().get();
        }
        tempReadyQueue.push(std::move(readyQueue.front()));
        readyQueue.pop();
    }
    while (!tempReadyQueue.empty()) {
        readyQueue.push(std::move(tempReadyQueue.front()));
        tempReadyQueue.pop();
    }

    return foundProcess;
}