#include "Memory.h"
#include <cmath>
#include <set>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "utils.h" 
#include "Frame.h"   
#include "Screen.h"



Memory::Memory(int mem_limit, int frame_mem)
	: max_mem(mem_limit), mem_per_frame(frame_mem) {

	num_of_frames = max_mem / mem_per_frame;

    int address = 0;
    for (int i = 0; i < num_of_frames; ++i) {
        Frame* f = new Frame();
        f->newFrame(mem_per_frame, address);
        mem_frames.push_back(f);
        address += mem_per_frame;
    }

}

void Memory::FF_AssignProcessToFrame(Screen* process) {
    int requiredMemory = process->getMemoryRequirement();
    int framesNeeded = (requiredMemory + mem_per_frame - 1) / mem_per_frame;

    std::vector<int> assignedFrames;
    for (int i = 0; i < mem_frames.size() && assignedFrames.size() < framesNeeded; ++i) {
        if (!mem_frames[i]->CheckIsOccupied()) {
            mem_frames[i]->Assign(process->getProcessName());
            assignedFrames.push_back(i);
        }
    }

    if (assignedFrames.size() < framesNeeded) {
        std::cout << "[Memory] Not enough memory to allocate process " << process->getProcessName() << "\n";
        for (int idx : assignedFrames) {
            mem_frames[idx]->EmptyFrame();
        }
        return;
    }

    for (int i = 0; i < assignedFrames.size(); ++i) {
        std::string varName = "VAR_" + std::to_string(i);
        int memAddr = assignedFrames[i] * mem_per_frame;
        process->addToSymbolTable(varName, memAddr);
    }

    std::cout << "[Memory] Process " << process->getProcessName() << " allocated " << framesNeeded << " frame(s).\n";
}



void Memory::FF_DetachProcessFromMemory(Screen* process) {
    string pname = process->getProcessName();
    for (Frame* f : mem_frames) {
        if (f->CheckContents() == pname) {
            f->EmptyFrame();
        }
    }
    process->clearSymbolTable();
    std::cout << "[Memory] Freed memory from process " << pname << "\n";
}



void Memory::GenerateMemoryReport(int quantumCycle) {
    std::ostringstream filename;
    filename << "memory_stamp_" << std::setw(4) << std::setfill('0') << quantumCycle << ".txt";
    std::ofstream file(filename.str());

    // Header
    file << "Timestamp: (" << getCurrentTimestamp() << ")\n";

    // Count processes
    std::set<std::string> uniqueProcesses;
    for (auto& frame : mem_frames) {
        if (frame.CheckIsOccupied()) {
            uniqueProcesses.insert(frame.CheckContents());
        }
    }
    file << "Number of processes in memory: " << uniqueProcesses.size() << "\n";

    // Calculate external fragmentation
    int externalFrag = 0;
    bool insideHole = false;
    int holeSize = 0;

    for (auto& frame : mem_frames) {
        if (!frame.CheckIsOccupied()) {
            externalFrag += mem_per_frame;
        }
    }
    file << "Total external fragmentation in KB: " << externalFrag << "\n\n";

    // Print ASCII Memory
    int address = max_mem;
    file << "----end---- = " << address << "\n";

    for (int i = num_of_frames - 1; i >= 0; i--) {
        address -= mem_per_frame;
        if (mem_frames[i].CheckIsOccupied()) {
            file << address + mem_per_frame << "\n";
            file << mem_frames[i].CheckContents() << "\n";
            file << address << "\n";
        }
    }

    file << "----start---- = 0\n";
    file.close();
}
  
void Memory::displayProcessSymbols(Screen* process) {
    std::cout << "Symbol Table for Process " << process->getPID() << ":\n";
    for (const auto& entry : process->getSymbolTable()) {
        std::cout << "  " << entry.first << " -> " << entry.second << "\n";
    }
}

void Memory::PrintMemoryToConsole(int quantumCycle) {
    cout << "=== MEMORY REPORT at Quantum " << quantumCycle << " ===" << endl;
    for (const auto& frame : mem_frames) {
        cout << "Frame " << frame->getAddress() << ": "
            << (frame->CheckIsOccupied() ? frame->CheckContents() : "[Empty]") << endl;
    }
    cout << "=======================================" << endl;
}
Memory::~Memory() {
    for (Frame* f : mem_frames) {
        delete f;
    }
    mem_frames.clear();
}