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
	struct Values {
		string process;
		int value;
	};
	std::vector<Frame> mem_frames;
	stack<int> free_frames;
	vector<Table> page_table;
    int max_mem;
    int num_of_frames;
    int mem_per_frame;
    std::vector<Values> memoryData; // actual memory to store int values

public:
	Memory(int mem_limit, int frame_mem);
	void LRU_AssignProcessToFrame(string process_index);
	void LRU_DetachProcessFromMemory(string process);
	void AddNewProcess(string name, vector<string> instructions, int process_size);
	void GenerateMemoryReport(int quantumCycle);
	void SetNextPageInProcess(string process);
	void MoveToNextInstruction(string process);
	bool VariableDeclaration(int value, string process_Name);
	int Read(int address, string process_Name) const {
        if (address < 0 || address >= max_mem)
            throw std::out_of_range("Memory read out of bounds");
		if (memoryData[address].process != process_Name)
			throw std::out_of_range("value address is not part of process");
        return memoryData[address].value;
    }

    void Write(int address, int value,  string process_Name) {
        if (address < 0 || address >= max_mem)
            throw std::out_of_range("Memory write out of bounds");
		if (memoryData[address].process != process_Name)
			throw std::out_of_range("value address is not part of process");
        memoryData[address].value = value;
    }
};

#endif
