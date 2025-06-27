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
#include <limits> // Required for numeric_limits
#include <memory> // Required for unique_ptr

#include "utils.h"

// Bring common C++ standard library elements into scope
using namespace std;

// Global configuration parameters
struct Config {
    int num_cpu = 4;
    string scheduler_type = "fcfs";
    int quantum_cycles = 5;
    int batch_process_freq = 1;
    unsigned int min_ins = 1000;
    unsigned int max_ins = 2000;
    int delay_per_exec = 0;
};

Config globalConfig; // Global config instance

// Forward declarations
class Process;
class BaseScheduler; // Base class for schedulers
class FCFSScheduler;
class RRScheduler;

// Global scheduler instance (using a raw pointer to BaseScheduler for polymorphism)
BaseScheduler* globalScheduler = nullptr;
atomic<bool> scheduler_initialized(false);

/**
* PROCESS CLASS
*/
class Process {
private:
    int id;
    unsigned int totalInstructions;
    unsigned int executedInstructions;
    string arrivalTimestamp;
    int coreAssigned;
    vector<string> printLogs;
    string name; // Human-readable name like screen_01
    atomic<bool> finished; // To indicate if the process has finished

public:
    // Constructor to initialize a new process
    Process(int newId, unsigned int newTotalInstructions, string timeArrived, const string& processName)
        : id(newId), totalInstructions(newTotalInstructions), arrivalTimestamp(timeArrived),
        executedInstructions(0), coreAssigned(-1), name(processName), finished(false) {}

    // Delete copy constructor and copy assignment operator for std::atomic member
    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;

    // Allow move constructor and move assignment operator
    Process(Process&& other) noexcept
        : id(other.id),
        totalInstructions(other.totalInstructions),
        executedInstructions(other.executedInstructions),
        arrivalTimestamp(std::move(other.arrivalTimestamp)),
        coreAssigned(other.coreAssigned),
        printLogs(std::move(other.printLogs)),
        name(std::move(other.name)),
        finished(other.finished.load()) { // Atomically load value for move
        other.id = 0; // Clear original
        other.totalInstructions = 0;
        other.executedInstructions = 0;
        other.coreAssigned = -1;
        other.finished.store(true); // Mark original as finished/invalidated
    }

    Process& operator=(Process&& other) noexcept {
        if (this != &other) {
            id = other.id;
            totalInstructions = other.totalInstructions;
            executedInstructions = other.executedInstructions;
            arrivalTimestamp = std::move(other.arrivalTimestamp);
            coreAssigned = other.coreAssigned;
            printLogs = std::move(other.printLogs);
            name = std::move(other.name);
            finished.store(other.finished.load()); // Atomically load and store

            other.id = 0; // Clear original
            other.totalInstructions = 0;
            other.executedInstructions = 0;
            other.coreAssigned = -1;
            other.finished.store(true); // Mark original as finished/invalidated
        }
        return *this;
    }


    void ExecuteInstruction(int coreNum);
    void AddPrintLog(const string& message, int coreNum);
    int GetPID() const;
    string GetName() const;
    string GetArrivalTime() const; // Renamed for clarity
    unsigned int GetExecutedInstructions() const; // Renamed for clarity
    unsigned int GetTotalInstructions() const;
    int GetCoreValue() const;
    void SetCoreValue(int value);
    vector<string> GetPrintLogs() const;
    bool IsFinished() const;
    void SetFinished(bool status);
};

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

/**
* BASE SCHEDULER CLASS
*/
class BaseScheduler {
public:
    virtual ~BaseScheduler() = default;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void SchedulerLoop() = 0;
    virtual void CreateProcess(bool isBatch = false, const string& userProvidedName = "") = 0; // Added userProvidedName
    virtual void DisplayStatus(ostream& os) = 0; // Modified to take ostream
    virtual bool IsRunning() = 0;
    // Modified GetProcessByName to search through finished processes as well
    // and to return nullptr if the process is finished, so the main thread
    // doesn't try to access a deallocated object.
    virtual Process* GetProcessByName(const string& name) = 0;
};

/**
* FCFS SCHEDULER CLASS
*/
class FCFSScheduler : public BaseScheduler {
private:
    vector<unique_ptr<Process>> cores; // Use unique_ptr
    queue<unique_ptr<Process>> readyQueue; // Use unique_ptr
    queue<unique_ptr<Process>> doneQueue; // Use unique_ptr
    mutex schedulerMutex;
    atomic<bool> running;
    atomic<int> currentPidInc;
    thread schedulerThread;
    atomic<long long> cpuCycles; // Use long long for cycles
    int numCores;
    unsigned int minInstructions;
    unsigned int maxInstructions;
    int batchProcessFrequency;
    int delayPerExecution;

public:
    FCFSScheduler(int num_cores, unsigned int min_ins, unsigned int max_ins, int batch_freq, int delay_exec);
    ~FCFSScheduler();
    void Start() override;
    void Stop() override;
    void SchedulerLoop() override;
    void CreateProcess(bool isBatch = false, const string& userProvidedName = "") override;
    void DisplayStatus(ostream& os) override;
    bool IsRunning() override;
    Process* GetProcessByName(const string& name) override;
};

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
    srand(static_cast<unsigned int>(time(0)));

    while (running) {
        {
            lock_guard<mutex> lock(schedulerMutex);

            cpuCycles++; // Increment CPU cycle

            // Process cores
            for (int i = 0; i < numCores; i++) {
                if (cores[i] && !cores[i]->IsFinished() && cores[i]->GetTotalInstructions() > 0) {
                    cores[i]->ExecuteInstruction(i);

                    if (delayPerExecution > 0) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(delayPerExecution)); // Simulate busy-waiting
                    }

                    // Check if process is done after executing an instruction
                    if (cores[i]->IsFinished()) {
                        doneQueue.push(std::move(cores[i])); // Move unique_ptr to done queue
                        // Assign next process from ready queue if available
                        if (!readyQueue.empty()) {
                            unique_ptr<Process> nextProcess = std::move(readyQueue.front()); // Move from ready queue
                            readyQueue.pop();
                            nextProcess->SetCoreValue(i);
                            cores[i] = std::move(nextProcess); // Move to core
                        }
                        else {
                            cores[i].reset(); // Core becomes idle (nullptr)
                        }
                    }
                }
            }

            // Generate new processes based on batchProcessFrequency
            if (batchProcessFrequency > 0 && cpuCycles % batchProcessFrequency == 0) {
                // Check if there's an available core
                bool assignedToCore = false;
                for (int i = 0; i < numCores; ++i) {
                    if (!cores[i]) { // Check if core is idle (nullptr)
                        string processName = "screen_" + string(2 - to_string(currentPidInc).length(), '0') + to_string(currentPidInc);
                        unsigned int instructions = minInstructions + (rand() % (maxInstructions - minInstructions + 1));
                        cores[i] = std::make_unique<Process>(currentPidInc, instructions, getCurrentTimestamp(), processName);
                        cores[i]->SetCoreValue(i);
                        currentPidInc++;
                        assignedToCore = true;
                        break;
                    }
                }
                if (!assignedToCore) {
                    // If no idle core, add to ready queue
                    string processName = "screen_" + string(2 - to_string(currentPidInc).length(), '0') + to_string(currentPidInc);
                    unsigned int instructions = minInstructions + (rand() % (maxInstructions - minInstructions + 1));
                    readyQueue.push(std::make_unique<Process>(currentPidInc, instructions, getCurrentTimestamp(), processName));
                    currentPidInc++;
                }
            }

            // Distribute processes from ready queue to idle cores if any
            for (int i = 0; i < numCores; ++i) {
                if (!cores[i] && !readyQueue.empty()) { // If core is idle and ready queue has processes
                    unique_ptr<Process> nextProcess = std::move(readyQueue.front());
                    readyQueue.pop();
                    nextProcess->SetCoreValue(i);
                    cores[i] = std::move(nextProcess);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep to prevent busy-waiting the main thread
    }
}

void FCFSScheduler::CreateProcess(bool isBatch, const string& userProvidedName) {
    lock_guard<mutex> lock(schedulerMutex);

    string processName;
    if (isBatch) {
        processName = "screen_" + string(2 - to_string(currentPidInc).length(), '0') + to_string(currentPidInc);
    }
    else {
        processName = userProvidedName;
    }

    unsigned int instructions = minInstructions + (rand() % (maxInstructions - minInstructions + 1));
    unique_ptr<Process> newProcess = std::make_unique<Process>(currentPidInc, instructions, getCurrentTimestamp(), processName);
    currentPidInc++;

    bool assigned = false;
    for (int i = 0; i < numCores; i++) {
        if (!cores[i]) { // If core is idle (nullptr)
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
    // Create a temporary queue to iterate through done processes without modifying the original
    queue<unique_ptr<Process>> tempDoneQueue;
    // Copy (move) processes from doneQueue to tempDoneQueue for display
    while (!doneQueue.empty()) {
        tempDoneQueue.push(std::move(doneQueue.front()));
        doneQueue.pop();
    }
    // Now iterate and print from tempDoneQueue, and then push them back to original doneQueue
    while (!tempDoneQueue.empty()) {
        const unique_ptr<Process>& p = tempDoneQueue.front(); // Use const reference
        os << p->GetName() << "    " << p->GetArrivalTime()
            << "    Finished    " << p->GetExecutedInstructions() << "/" << p->GetTotalInstructions() << endl;
        doneQueue.push(std::move(tempDoneQueue.front())); // Move back to original queue
        tempDoneQueue.pop();
    }
    os << "--------------------------------" << endl;
}

bool FCFSScheduler::IsRunning() {
    return running.load(); // Use load for atomic boolean
}

Process* FCFSScheduler::GetProcessByName(const string& name) {
    lock_guard<mutex> lock(schedulerMutex);
    // Search in running processes (cores)
    for (int i = 0; i < numCores; ++i) {
        if (cores[i] && cores[i]->GetName() == name && !cores[i]->IsFinished()) {
            return cores[i].get(); // Return raw pointer if found and not finished
        }
    }
    // If not found in running cores, search in ready queue
    // Note: Iterating std::queue directly is not possible.
    // A more efficient way for searching would be to use a std::list or std::vector for the ready queue
    // if frequent searching is needed, or move to std::map<string, unique_ptr<Process>> for lookup.
    // For now, iterate by temporary moving to check.
    queue<unique_ptr<Process>> tempReadyQueue;
    Process* foundProcess = nullptr;

    while (!readyQueue.empty()) {
        if (readyQueue.front()->GetName() == name && !readyQueue.front()->IsFinished()) {
            // Found in ready queue, but we cannot return a raw pointer that would become invalid if moved later.
            // For now, we will not return a pointer from the ready queue for 'screen -r'
            // as it implies interaction with a 'currently active' screen.
            // The specification implies 'screen -r' is for *running* processes.
            // If the spec implies finished processes can be viewed, you'd iterate doneQueue too.
            // Given the original context of the error, we focus on the running state.
            foundProcess = readyQueue.front().get(); // Get raw pointer, but be careful with its lifetime
            // This will still be problematic if the process gets assigned to a core.
            // The safer approach is to only return if it's currently on a core.
            // For now, let's keep the logic to only check active cores for `screen -r`
            // as per the implied "running" context for screen -r.
        }
        tempReadyQueue.push(std::move(readyQueue.front()));
        readyQueue.pop();
    }
    while (!tempReadyQueue.empty()) {
        readyQueue.push(std::move(tempReadyQueue.front()));
        tempReadyQueue.pop();
    }

    // Search in finished processes (doneQueue) - this is for completeness,
    // but the 'screen -r' command generally implies *running* processes.
    // If a process has finished, it might not be relevant to "resume" its screen.
    // The prompt says: "If the process name is not found/finished execution, the console prints "Process <process name> not found."
    // This implies that 'screen -r' should NOT return finished processes.
    // So, we only return from active cores.

    return nullptr; // Not found in running cores (or finished)
}


/**
* RR SCHEDULER CLASS
*/
class RRScheduler : public BaseScheduler {
private:
    struct CoreSlot {
        unique_ptr<Process> proc; // Use unique_ptr
        int qRemaining;
        bool isEmpty;

        CoreSlot() : qRemaining(0), isEmpty(true) {}
    };

    vector<CoreSlot> cores;
    queue<unique_ptr<Process>> readyQueue; // Use unique_ptr
    queue<unique_ptr<Process>> doneQueue; // Use unique_ptr
    mutex schedulerMutex;
    atomic<bool> running;
    atomic<int> currentPidInc;
    thread schedulerThread;
    atomic<long long> cpuCycles; // Use long long for cycles
    int numCores;
    int quantum;
    unsigned int minInstructions;
    unsigned int maxInstructions;
    int batchProcessFrequency;
    int delayPerExecution;

public:
    RRScheduler(int num_cores, int quantum_cycles, unsigned int min_ins, unsigned int max_ins, int batch_freq, int delay_exec);
    ~RRScheduler();
    void Start() override;
    void Stop() override;
    void SchedulerLoop() override;
    void CreateProcess(bool isBatch = false, const string& userProvidedName = "") override;
    void DisplayStatus(ostream& os) override;
    bool IsRunning() override;
    Process* GetProcessByName(const string& name) override;
};

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
    srand(static_cast<unsigned int>(time(0)));

    while (running) {
        {
            lock_guard<mutex> lock(schedulerMutex);

            cpuCycles++; // Increment CPU cycle

            // Process cores
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
                            unique_ptr<Process> nextProcess = std::move(readyQueue.front());
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
                            unique_ptr<Process> nextProcess = std::move(readyQueue.front());
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
                        unique_ptr<Process> newProcess = std::make_unique<Process>(currentPidInc, instructions, getCurrentTimestamp(), processName);
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
                    readyQueue.push(std::make_unique<Process>(currentPidInc, instructions, getCurrentTimestamp(), processName));
                    currentPidInc++;
                }
            }

            // Distribute processes from ready queue to idle cores if any
            for (int i = 0; i < numCores; ++i) {
                if (cores[i].isEmpty && !readyQueue.empty()) {
                    unique_ptr<Process> nextProcess = std::move(readyQueue.front());
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
    unique_ptr<Process> newProcess = std::make_unique<Process>(currentPidInc, instructions, getCurrentTimestamp(), processName);
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
    queue<unique_ptr<Process>> tempDoneQueue;
    while (!doneQueue.empty()) {
        tempDoneQueue.push(std::move(doneQueue.front()));
        doneQueue.pop();
    }
    while (!tempDoneQueue.empty()) {
        const unique_ptr<Process>& p = tempDoneQueue.front();
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

Process* RRScheduler::GetProcessByName(const string& name) {
    lock_guard<mutex> lock(schedulerMutex);
    for (int i = 0; i < numCores; ++i) {
        if (!cores[i].isEmpty && cores[i].proc && cores[i].proc->GetName() == name && !cores[i].proc->IsFinished()) {
            return cores[i].proc.get(); // Return raw pointer
        }
    }
    return nullptr; // Not found in running cores
}


/**
* COMMAND FUNCTIONS
*/

// Function to read config.txt
bool readConfig(Config& config) {
    ifstream configFile("config.txt");
    if (!configFile.is_open()) {
        printColor("Error: Failed to open config.txt. Using default parameters.\n", RED);
        return false;
    }

    string line;
    while (getline(configFile, line)) {
        stringstream ss(line);
        string paramName;
        ss >> paramName;

        if (paramName == "num-cpu") {
            ss >> config.num_cpu;
        }
        else if (paramName == "scheduler") {
            ss >> std::quoted(config.scheduler_type); // Use std::quoted for strings with spaces
        }
        else if (paramName == "quantum-cycles") {
            ss >> config.quantum_cycles;
        }
        else if (paramName == "batch-process-freq") {
            ss >> config.batch_process_freq;
        }
        else if (paramName == "min-ins") {
            ss >> config.min_ins;
        }
        else if (paramName == "max-ins") {
            ss >> config.max_ins;
        }
        else if (paramName == "delay-per-exec") {
            ss >> config.delay_per_exec;
        }
    }
    configFile.close();
    printColor("Config loaded successfully from config.txt.\n", GREEN);
    return true;
}

void initialize() {
    printColor("\"initialize\" command recognized. Initializing scheduler...\n", YELLOW);
    if (scheduler_initialized.load()) { // Use load for atomic boolean
        printColor("Scheduler already initialized. Please stop it first if you want to re-initialize.\n", YELLOW);
        return;
    }

    if (readConfig(globalConfig)) { // Read config parameters
        if (globalConfig.scheduler_type == "fcfs") {
            globalScheduler = new FCFSScheduler(globalConfig.num_cpu, globalConfig.min_ins, globalConfig.max_ins, globalConfig.batch_process_freq, globalConfig.delay_per_exec);
            printColor("FCFS Scheduler configured.\n", GREEN);
        }
        else if (globalConfig.scheduler_type == "rr") {
            globalScheduler = new RRScheduler(globalConfig.num_cpu, globalConfig.quantum_cycles, globalConfig.min_ins, globalConfig.max_ins, globalConfig.batch_process_freq, globalConfig.delay_per_exec);
            printColor("Round Robin Scheduler configured (Quantum: " + to_string(globalConfig.quantum_cycles) + ").\n", GREEN);
        }
        else {
            printColor("Invalid scheduler type specified in config.txt. Defaulting to FCFS.\n", RED);
            globalScheduler = new FCFSScheduler(globalConfig.num_cpu, globalConfig.min_ins, globalConfig.max_ins, globalConfig.batch_process_freq, globalConfig.delay_per_exec);
        }
        globalScheduler->Start();
        scheduler_initialized.store(true); // Use store for atomic boolean
        printColor("Scheduler initialized with " + to_string(globalConfig.num_cpu) + " cores.\n", GREEN);
    }
    else {
        printColor("Failed to load config. Using default scheduler (FCFS) and parameters.\n", YELLOW);
        globalScheduler = new FCFSScheduler(globalConfig.num_cpu, globalConfig.min_ins, globalConfig.max_ins, globalConfig.batch_process_freq, globalConfig.delay_per_exec);
        globalScheduler->Start();
        scheduler_initialized.store(true); // Use store for atomic boolean
    }
}

void schedulerStart() { // Renamed from schedulerTest
    printColor("\"scheduler-start\" command recognized. Starting continuous process generation...\n", YELLOW);
    if (globalScheduler != nullptr && globalScheduler->IsRunning()) {
        // The scheduler's loop itself handles continuous process generation based on batchProcessFrequency
        printColor("Scheduler is generating processes. Use 'screen -ls' to see active processes.\n", GREEN);
    }
    else {
        printColor("Scheduler not initialized or not running. Please run 'initialize' first.\n", RED);
    }
}

void schedulerStop() {
    printColor("\"scheduler-stop\" command recognized. Stopping scheduler...\n", YELLOW);
    if (globalScheduler != nullptr) {
        globalScheduler->Stop();
        delete globalScheduler;
        globalScheduler = nullptr;
        scheduler_initialized.store(false); // Use store for atomic boolean
        printColor("Scheduler stopped.\n", GREEN);
    }
    else {
        printColor("Scheduler not running.\n", YELLOW);
    }
}

void reportUtil() {
    printColor("\"report-util\" command recognized. Generating report...\n", YELLOW);
    if (globalScheduler != nullptr && globalScheduler->IsRunning()) {
        // Display to console
        printColor("\n--- CPU Utilization Report (Console) ---\n", CYAN);
        globalScheduler->DisplayStatus(cout);

        // Save to file
        ofstream logFile("csopesy-log.txt");
        if (logFile.is_open()) {
            globalScheduler->DisplayStatus(logFile);
            logFile.close();
            printColor("Report generated and saved to csopesy-log.txt!\n", GREEN);
        }
        else {
            printColor("Error: Unable to open csopesy-log.txt for writing.\n", RED);
        }
    }
    else {
        printColor("Scheduler not initialized or not running. Please run 'initialize' first.\n", RED);
    }
}

/**
* MAIN FUNCTION
*/
int main() {
    // For ensuring consistent output on Windows console for colors
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD consoleMode;
    GetConsoleMode(hConsole, &consoleMode);
    SetConsoleMode(hConsole, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif

    // Initial program display: print banner and subtitle once
    printBanner();
    printSubtitle();

    while (true) {
        // The screen will now scroll for general commands.
        // Clearing is handled explicitly by 'clear' command or 'screen -r' mode transitions.

        string command;
        printColor("\n~> ", GREEN); // Added newline for better spacing with scrolling
        getline(cin, command);

        // Trim whitespace from command
        command.erase(0, command.find_first_not_of(" \t\n\r\f\v"));
        command.erase(command.find_last_not_of(" \t\n\r\f\v") + 1);


        if (command == "help") {
            printHelp();
        }
        else if (command == "initialize") {
            initialize();
            // No screen clear needed here, output will just follow.
        }
        else if (!scheduler_initialized.load() && command != "exit") { // Only 'exit' is recognized before 'initialize'
            printColor("Please run 'initialize' first before executing other commands.\n", RED);
        }
        else if (command.find("screen") == 0) { // Command starts with "screen"
            if (command.find("-s") != string::npos) { // Create new screen/process
                string processName = command.substr(command.find("-s") + 3); // Get name after "-s "
                if (!processName.empty()) {
                    if (globalScheduler) {
                        Process* existingProcess = globalScheduler->GetProcessByName(processName); // Checks running processes
                        if (existingProcess) {
                            printColor("Process '" + processName + "' is already running.\n", MAGENTA);
                        }
                        else {
                            globalScheduler->CreateProcess(false, processName); // Create a single non-batch process with user-provided name
                            printColor("Screen/Process '" + processName + "' created. Use 'screen -ls' to see it.\n", GREEN);
                        }
                    }
                    else {
                        printColor("Scheduler not initialized. Cannot create a process.\n", RED);
                    }
                }
                else {
                    printColor("Invalid screen name. Usage: screen -s <name>\n", RED);
                }
            }
            else if (command.find("-r") != string::npos) { // Resume existing screen/process
                string processName = command.substr(command.find("-r") + 3); // Get name after "-r "
                if (!processName.empty()) {
                    if (globalScheduler) {
                        // RE-FETCH THE PROCESS POINTER INSIDE THE LOOP FOR SAFETY
                        // This ensures 'targetProcess' always points to a valid, currently active object.
                        Process* targetProcess = globalScheduler->GetProcessByName(processName);

                        if (targetProcess) { // Check if process is currently running/active in cores
                            // We explicitly check IsFinished() again inside the loop for robustness
                            // although GetProcessByName should ideally not return finished processes.
                            if (targetProcess->IsFinished()) { // This check becomes more critical if GetProcessByName ever returns a finished process
                                printColor("Process '" + processName + "' has finished execution.\n", YELLOW);
                            }
                            else {
                                // Initial display of the process screen
                                system("cls");
                                printBanner();
                                printColor("\n--- Process Screen: " + targetProcess->GetName() + " ---\n", YELLOW);
                                printColor("ID: " + to_string(targetProcess->GetPID()) + "\n", WHITE);
                                printColor("Logs:\n", WHITE);
                                for (const string& log : targetProcess->GetPrintLogs()) {
                                    cout << log << endl;
                                }
                                printColor("Current instruction line: " + to_string(targetProcess->GetExecutedInstructions()) + "\n", WHITE);
                                printColor("Lines of code: " + to_string(targetProcess->GetTotalInstructions()) + "\n", WHITE);

                                printPlaceHolderConsoles(); // Display placeholder console options

                                while (true) { // Loop for process-specific commands
                                    string process_command;
                                    printColor("~> process-cli> ", GREEN);
                                    getline(cin, process_command);

                                    process_command.erase(0, process_command.find_first_not_of(" \t\n\r\f\v"));
                                    process_command.erase(process_command.find_last_not_of(" \t\n\r\f\v") + 1);

                                    // Re-fetch targetProcess inside the loop to ensure it's still valid
                                    Process* currentProcessState = globalScheduler->GetProcessByName(processName);

                                    if (!currentProcessState) {
                                        // Process is no longer running or was deallocated/moved.
                                        printColor("Process '" + processName + "' is no longer active or has finished.\n", RED);
                                        break; // Exit process screen loop
                                    }

                                    if (process_command == "process-smi") { // Print simple information about process
                                        system("cls"); // Clear and redraw
                                        printBanner();
                                        printColor("\n--- Process Screen: " + currentProcessState->GetName() + " ---\n", YELLOW);
                                        printColor("ID: " + to_string(currentProcessState->GetPID()) + "\n", WHITE);
                                        printColor("Logs:\n", WHITE);
                                        for (const string& log : currentProcessState->GetPrintLogs()) {
                                            cout << log << endl;
                                        }
                                        if (currentProcessState->IsFinished()) {
                                            printColor("Finished!\n", YELLOW);
                                        }
                                        else {
                                            printColor("Current instruction line: " + to_string(currentProcessState->GetExecutedInstructions()) + "\n", WHITE);
                                            printColor("Lines of code: " + to_string(currentProcessState->GetTotalInstructions()) + "\n", WHITE);
                                        }
                                        printPlaceHolderConsoles();
                                    }
                                    else if (process_command == "exit") { // Return to main menu
                                        break; // Exit process screen loop
                                    }
                                    else {
                                        printColor("Unknown command within process screen.\n", RED);
                                    }
                                }
                                // Clear screen and re-print main menu header when exiting process screen
                                system("cls");
                                printBanner();
                                printSubtitle();
                            }
                        }
                        else {
                            printColor("Process '" + processName + "' not found or has finished execution.\n", MAGENTA);
                        }
                    }
                    else {
                        printColor("Scheduler not initialized. Cannot access processes.\n", RED);
                    }
                }
                else {
                    printColor("Invalid screen name. Usage: screen -r <name>\n", RED);
                }
            }
            else if (command.find("-ls") != string::npos) { // List all running processes
                if (globalScheduler) {
                    globalScheduler->DisplayStatus(cout); // Display status to console
                }
                else {
                    printColor("Scheduler not initialized. No active processes to list.\n", CYAN);
                }
            }
            else {
                printColor("Screen command not recognized. Usage: screen -s <name>, screen -r <name>, or screen -ls\n", RED);
            }
        }
        else if (command == "scheduler-start") { // Formerly scheduler-test
            schedulerStart();
        }
        else if (command == "scheduler-stop") {
            schedulerStop();
        }
        else if (command == "report-util") {
            reportUtil();
        }
        else if (command == "clear") {
            clear(); // Explicit clear command
            printSubtitle(); // Re-print subtitle after clear (banner is handled by clear())
        }
        else if (command == "exit") {
            printColor("Exiting...\n", RED);
            if (globalScheduler != nullptr) {
                schedulerStop();
            }
            break; // Exit the main loop
        }
        else {
            printColor("Unknown command. Type 'help' for a list of commands.\n", RED);
        }
    }

    return 0;
}
