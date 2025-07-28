#ifndef FRAME_H
#define FRAME_H
#include <iostream>
#include <string>

#include "Screen.h"

using namespace std;

#ifndef FRAME_H
#define FRAME_H

class Frame {
	int frame_mem;
	int address = 0;
	string process;
	bool is_occupied;
	int lower_bound;
	int upper_bound;
	std::string contents;

public:
	Frame() : frame_mem(0), address(0), process(""), is_occupied(false) {}
	void newFrame(int frameMem, int frame_add);
	void AssignProcess(string process_name);
	bool CheckIsOccupied();
	void EmptyFrame();

	void newFrame(int size, int addr);

	bool CheckIsOccupied();

	void Assign(std::string processName);

	void EmptyFrame();

	std::string CheckContents();

	int getAddress();


	string CheckContents();
};

#endif
#endif
