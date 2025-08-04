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