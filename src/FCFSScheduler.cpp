using namespace std;

#include "FCFSScheduler.h"
#include "utils.h"
#include "Memory.h"

string padNumberFCFS(int number, int width) {
    string numStr = to_string(number);
    if (numStr.length() >= width)
        return numStr;
    return string(width - numStr.length(), '0') + numStr;
}


FCFSScheduler::FCFSScheduler(int num_cores, unsigned int min_ins, unsigned int max_ins, int batch_freq, int delay_exec, Memory* mem)
    : running(false), currentPidInc(1), cpuCycles(0), numCores(num_cores),
    minInstructions(min_ins), maxInstructions(max_ins), batchProcessFrequency(batch_freq),
    delayPerExecution(delay_exec) {
    cores.resize(numCores);

    // Use provided memory or create new one
    if (mem) {
        memory = mem;
    }
    else {
        memory = new Memory(16384, 4096); // Use config values
    }
}

FCFSScheduler::~FCFSScheduler() {
    Stop();
    delete memory; // Clean up memory
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
    srand(static_cast<unsigned int>(time(0)));

    while (running) {
        {
            lock_guard<mutex> lock(schedulerMutex);

            cpuCycles++; // Increment CPU cycle

            // Screen cores
            for (int i = 0; i < numCores; i++) {
                if (cores[i] && !cores[i]->IsFinished() && cores[i]->GetTotalInstructions() > 0) {
                    cores[i]->ExecuteInstruction(i);

                    if (delayPerExecution > 0) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(delayPerExecution)); // Simulate busy-waiting
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

            if (enableBatchFlag && batchProcessFrequency > 0 && cpuCycles % batchProcessFrequency == 0) {
                bool assignedToCore = false;
                for (int i = 0; i < numCores; ++i) {
                    if (!cores[i]) {
                        string processName = "screen_" + padNumberFCFS(currentPidInc, 2);
                        unsigned int instructions = minInstructions + (rand() % (maxInstructions - minInstructions + 1));
                        cores[i] = std::make_unique<Screen>(currentPidInc, instructions, getCurrentTimestamp(), processName);
                        cores[i]->SetCoreValue(i);
                        currentPidInc++;
                        assignedToCore = true;
                        break;
                    }
                }
                if (!assignedToCore) {
                    string processName = "screen_" + padNumberFCFS(currentPidInc, 2);
                    unsigned int instructions = minInstructions + (rand() % (maxInstructions - minInstructions + 1));
                    readyQueue.push(std::make_unique<Screen>(currentPidInc, instructions, getCurrentTimestamp(), processName));
                    currentPidInc++;
                }
            }

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
        string processName = "screen_" + padNumberFCFS(currentPidInc, 2);
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

    return nullptr;
}

void FCFSScheduler::SetBatchEnabled(bool enabled) {
    enableBatchFlag.store(enabled);
}

void FCFSScheduler::CreateProcessWithInstructions(const string& processName, int processSize, const vector<string>& instructionList) {
    lock_guard<mutex> lock(schedulerMutex);

    if (instructionList.empty() || instructionList.size() > 50) {
        printColor("Invalid command: Instruction count must be between 1 and 50.\n", RED);
        return;
    }

    // Check if process already exists
    for (int i = 0; i < numCores; i++) {
        if (cores[i] && cores[i]->GetName() == processName) {
            printColor("Process '" + processName + "' already exists.\n", RED);
            return;
        }
    }

    // Check ready queue as well
    queue<unique_ptr<Screen>> tempReadyQueue;
    bool processExists = false;
    while (!readyQueue.empty()) {
        if (readyQueue.front()->GetName() == processName) {
            processExists = true;
        }
        tempReadyQueue.push(std::move(readyQueue.front()));
        readyQueue.pop();
    }
    while (!tempReadyQueue.empty()) {
        readyQueue.push(std::move(tempReadyQueue.front()));
        tempReadyQueue.pop();
    }

    if (processExists) {
        printColor("Process '" + processName + "' already exists in ready queue.\n", RED);
        return;
    }

    // Create the process with proper constructor parameters
    // Assuming Screen constructor takes: (pid, totalInstructions, arrivalTime, name, customInstructions, memory)
    unique_ptr<Screen> newProcess = make_unique<Screen>(
        processName,                    // Process name
        this->memory,                   // Memory object
        instructionList                 // Custom instructions
    );
    newProcess->SetPID(currentPidInc.load());

    currentPidInc++; // Don't forget to increment PID counter

    // Try to assign to an available core
    bool assigned = false;
    for (int i = 0; i < numCores; i++) {
        if (!cores[i]) {
            newProcess->SetCoreValue(i);
            cores[i] = std::move(newProcess);
            assigned = true;
            break;
        }
    }

    // If no core available, add to ready queue
    if (!assigned) {
        readyQueue.push(std::move(newProcess));
    }
}

void FCFSScheduler::DisplayMemoryStats() {
    memory->GenerateMemoryReport(cpuCycles);
}



