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
#include "Process.h"
#include "FCFSScheduler.h"
#include "RRScheduler.h"
#include "Session.h"
#include "BaseScheduler.h"

// Bring common C++ standard library elements into scope
using namespace std;

// Global scheduler instance
Config globalScheduler;

//array of sessions for individual screen
Session sessions[500];

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
