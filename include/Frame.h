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

public:
	void newFrame(int frameMem, int frame_add);
	void AssignProcess(string process_name);
	bool CheckIsOccupied();
	void EmptyFrame();
	string CheckContents();
};

#endif
