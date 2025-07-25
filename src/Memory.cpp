#include "Memory.h"
#include <set>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "utils.h" 
#include "Frame.h"   

Memory::Memory(int mem_limit, int frame_mem)
    : max_mem(mem_limit), mem_per_frame(frame_mem) {
    num_of_frames = max_mem / mem_per_frame;
    mem_frames.resize(num_of_frames);

    int address = mem_per_frame;

    for (int i = 0; i < num_of_frames; i++) {
        mem_frames[i].newFrame(mem_per_frame, address);
        address = address + mem_per_frame;
    }
}

void Memory::FF_AssignProcessToFrame(Screen* process, int processSize) {
    int mem_to_alloc = processSize / mem_per_frame;

    int i = 0, j = 0;

    while (i < num_of_frames) {
        int slotFound = 0;
        j = i;
        while (slotFound < mem_to_alloc && j < num_of_frames) {
            if (mem_frames[j].CheckIsOccupied()) break;
            slotFound++;
            j++;
        }

        if (slotFound == mem_to_alloc) {
            for (int k = 0; k < mem_to_alloc; k++, i++) {
                mem_frames[i].AssignProcess(process);
            }
            return;
        }

        i++;
    }
}

void Memory::FF_DetachProcessFromMemory(Screen* process) {
    for (int i = 0; i < num_of_frames; i++) {
        if (mem_frames[i].CheckContents() == process) {
            mem_frames[i].EmptyFrame();
        }
    }
}

void Memory::GenerateMemoryReport(int quantumCycle) {
    std::ostringstream filename;
    filename << "memory_stamp_" << std::setw(2) << std::setfill('0') << quantumCycle << ".txt";
    std::ofstream file(filename.str());

    file << "Timestamp: (" << getCurrentTimestamp() << ")\n";

    std::set<Screen*> uniqueProcesses;
    for (auto& frame : mem_frames) {
        if (frame.CheckIsOccupied()) {
            uniqueProcesses.insert(frame.CheckContents());
        }
    }
    file << "Number of processes in memory: " << uniqueProcesses.size() << "\n";

    int externalFrag = 0;
    for (auto& frame : mem_frames) {
        if (!frame.CheckIsOccupied()) {
            externalFrag += mem_per_frame;
        }
    }
    file << "Total external fragmentation in KB: " << externalFrag << "\n\n";

    int address = max_mem;
    file << "----end---- = " << address << "\n";

    for (int i = num_of_frames - 1; i >= 0; i--) {
        address -= mem_per_frame;
        if (mem_frames[i].CheckIsOccupied()) {
            file << address + mem_per_frame << "\n";
            file << mem_frames[i].CheckContents()->GetProcessName() << "\n";
            file << address << "\n";
        }
    }

    file << "----start---- = 0\n";
    file.close();
}
