#include "Page.h"
#include <set>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "utils.h"

Page::Page(string name_assign, vector<string> list, int list_size, int mem_page_size) 
	: process_name(name_assign), instruction_list(list), page_size(mem_page_size) {
	current_instruction = 0;
	current_frame = -1;
}

void Page::SetCurrentInstruction() {
	if (current_instruction < instruction_list.size()) {
		current_instruction++;
	}
}

string Page::CheckCurrentInstruction() {
	return instruction_list[current_instruction];
}

string Page::GetProcessAssigned() {
	return process_name;
}

void Page::SetCurrentFrame(int frame_no) {
	current_frame = frame_no;
	use_count++;
}

int Page::GetPageTally() {
	return use_count;
}

int Page::GetCurrentInstructionLine() {
	return current_instruction;
}

bool Page::IsPageInstructionsDone() {
	if (current_instruction == instruction_list.size()) {
		return true;
	}

	return false;
}