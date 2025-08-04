#include "Memory.h"
#include <set>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "utils.h" 
#include "Frame.h"   
#include <unordered_map>
#include <mutex>
#include <cstdint>

std::unordered_map<uint32_t, uint16_t> globalMemory;
std::mutex globalMemoryMutex;
const uint32_t MEMORY_MIN_ADDRESS = 0x1000;
const uint32_t MEMORY_MAX_ADDRESS = 0xFFFF; // Example: 64KB address space

Memory::Memory(int mem_limit, int frame_mem)
	: max_mem(mem_limit), mem_per_frame(frame_mem) {
	num_of_frames = max_mem / mem_per_frame;

    memoryData.resize(max_mem, 0);

	mem_frames.resize(num_of_frames);
	int address = mem_per_frame;

	for (int i = 0; i < num_of_frames; i++) {
		mem_frames[i].newFrame(mem_per_frame, address);
		address = address + mem_per_frame;
	}
}

void Memory::FF_AssignProcessToFrame(string process, int processSize) {
	int mem_to_alloc = processSize / mem_per_frame;

	int i = 0, j = 0;

	while (i < num_of_frames) {
		int slotFound = 0;
		j = i;
		while (slotFound < mem_to_alloc) {
			if (mem_frames[j].CheckIsOccupied()) {
				break;
			}
			slotFound++;
			j++;
		}

		if (slotFound == mem_to_alloc) {
			int k = 0;
			while (k < mem_to_alloc) {
				mem_frames[i].AssignProcess(process);
				i++;
			}
			break;
		}

		i++;
	}
}

void Memory::FF_DetachProcessFromMemory(string process) {
	for (int i = 0; i < num_of_frames; i++) {
		if (process == mem_frames[i].CheckContents()) {
			mem_frames[i].EmptyFrame();
		}
	}
}

void Memory::LRU_AssignProcessToFrame(string process_index) {
	int i = 0;
	Table page_to_assign;
	bool isAssigned = false;
	bool process_found = false;
	int j = 0;

	for (Table page : page_table) {
		if (page.process == process_index) {
			page_to_assign = page;
			break;
		}
		j++;
	}

	//find an empty memory frame
	while (i < num_of_frames) {
		if (!mem_frames[i].CheckIsOccupied()) {
			mem_frames[i].AssignProcess(page_to_assign.process, &page_to_assign.pages[page_to_assign.current_Page]);
			page_to_assign.pages[page_to_assign.current_Page].SetCurrentFrame(i);
			isAssigned = true;
			page_table[j].assigned_address = mem_frames[i].GetFrameAddress();
			break;
		}

		i++;
	}

	//if all occupied, find frame with least recently used process page
	if (!isAssigned) {
		int less_used_process = 0;
		int i = 1;

		while (i < num_of_frames) {
			if (mem_frames[less_used_process].ReturnPageTally() > mem_frames[i].ReturnPageTally()) {
				less_used_process = i;
			}
			i++;
		}

		LRU_DetachProcessFromMemory(mem_frames[less_used_process].CheckContents());

		mem_frames[less_used_process].AssignProcess(page_to_assign.process, &page_to_assign.pages[page_to_assign.current_Page]);
		page_to_assign.pages[page_to_assign.current_Page].SetCurrentFrame(less_used_process);
		page_table[j].assigned_address = mem_frames[less_used_process].GetFrameAddress();
	}
}

void Memory::LRU_DetachProcessFromMemory(string process) {
	int j = 0;

	for (Table page : page_table) {
		if (page.process == process) {
			break;
		}
		j++;
	}

	if (!page_table[j].pages[page_table[j].current_Page].IsPageInstructionsDone()) {
		ofstream backing_store("csopesy-backing-store.txt");

		backing_store << page_table[j].process << " - " << page_table[j].current_Page << "\n";

		backing_store.close();
	}
}

void Memory::AddNewProcess(string name, vector<string> instructions, int process_size) {
	Table newRow;
	int number_of_pages = process_size / mem_per_frame;
	int instructions_per_page = instructions.size() / number_of_pages;
	newRow.process = name;
	vector<string> temp;
	int i = 0;
	int j = 0;
	while (i < number_of_pages) {
		while (j < instructions.size()) {
			if (j % instructions_per_page > 0) {
				temp.push_back(instructions[j]);
			}
			else if (j % instructions_per_page == 0 || j == instructions.size() - 1)
			{
				break;
			}
			j++;
		}

		Page temp_Page = Page(name, temp, instructions_per_page, mem_per_frame);

		newRow.pages.push_back(temp_Page);

		temp.clear();
	}
}

void Memory::GenerateMemoryReport(int quantumCycle) {
    std::ostringstream filename;
    filename << "memory_stamp_" << std::setw(2) << std::setfill('0') << quantumCycle << ".txt";
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

void Memory::SetNextPageInProcess(string process) {
	int j = 0;

	for (Table page : page_table) {
		if (page.process == process) {
			break;
		}
		j++;
	}

	if (page_table[j].current_Page < page_table[j].pages.size()) {
		if (page_table[j].pages[page_table[j].current_Page].IsPageInstructionsDone()) {
			page_table[j].current_Page++;
		}
	}
}

void Memory::MoveToNextInstruction(string process) {
	int j = 0;

	for (Table page : page_table) {
		if (page.process == process) {
			break;
		}
		j++;
	}

	if (page_table[j].assigned_address > 0) {
		page_table[j].pages[page_table[j].current_Page].SetCurrentInstruction();
	}
}