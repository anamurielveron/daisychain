#ifndef MEMORY_H
#define MEMORY_H

#include <iostream>
#include <string>
#include <stack>

#include <vector>
#include <stdexcept>

#include "Frame.h"
#include "Page.h"

using namespace std;

extern const uint32_t MEMORY_MIN_ADDRESS;
extern const uint32_t MEMORY_MAX_ADDRESS;
class Screen;



class Memory {
private:
	struct Table {
			string process;
			vector<Page> pages;
			int assigned_address;
			int current_Page = 0;;
		};
	std::vector<Frame> mem_frames;
	stack<int> free_frames;
	vector<Table> page_table;
    int max_mem;
    int num_of_frames;
    int mem_per_frame;
    std::vector<int> memoryData; // actual memory to store int values

public:
	Memory(int mem_limit, int frame_mem);
	void LRU_AssignProcessToFrame(string process_index);
	void LRU_DetachProcessFromMemory(string process);
	void AddNewProcess(string name, vector<string> instructions, int process_size);
	void GenerateMemoryReport(int quantumCycle);
	void SetNextPageInProcess(string process);
	void MoveToNextInstruction(string process);
	int Read(int address) const {
        if (address < 0 || address >= max_mem)
            throw std::out_of_range("Memory read out of bounds");
        return memoryData[address];
    }

    void Write(int address, int value) {
        if (address < 0 || address >= max_mem)
            throw std::out_of_range("Memory write out of bounds");
        memoryData[address] = value;
    }
};

#endif
