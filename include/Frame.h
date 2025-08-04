#include <iostream>
#include <string>

#include "Screen.h"
#include "Page.h"


using namespace std;

#ifndef FRAME_H
#define FRAME_H

class Frame {
	int frame_mem = 0;
	int address = 0;
	string current_process;
	string process;
	bool is_occupied;
	string current_instruction;
	Page* assigned_Page;
	int lower_bound = 0;
	int upper_bound = 0;
	std::string contents;


public:
	void newFrame(int frameMem, int frame_add);
	void AssignProcess(string process_name, Page* page);
	bool CheckIsOccupied();
	void EmptyFrame();
	string CheckContents();
	int GetFrameAddress() const;
};

#endif
