#include <iostream>
#include <string>
#include <stack>

#include <vector>

using namespace std;

#ifndef PAGE_H
#define PAGE_H

class Page {
	string process_name;
	vector<string> instruction_list;

	int page_size;
	int current_instruction;
	int current_frame;
	int use_count = 0;

public:
	Page(string name_assign, vector<string> list, int list_size, int mem_page_size);
	void SetCurrentInstruction();
	string CheckCurrentInstruction();
	string GetProcessAssigned();
	void SetCurrentFrame(int frame_no);
	int GetPageTally();
	int GetCurrentInstructionLine();
	bool IsPageInstructionsDone();
};

#endif