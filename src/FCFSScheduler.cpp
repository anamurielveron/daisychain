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