#ifndef FRAME_H
#define FRAME_H

#include <iostream>
#include <string>
#include "Screen.h"
#include "Page.h"

using namespace std;

class Frame {
    int frame_mem = 0;
    int address = 0;
    string current_process;
    string process;
    bool is_occupied = false;
    int lower_bound;
    int upper_bound;
    string current_instruction;
    Page* assigned_Page = nullptr;

public:
    Frame() = default;
    void newFrame(int frameMem, int frame_add);
    void AssignProcess(string process_name, Page* page);
    bool CheckIsOccupied() const;
    void EmptyFrame();
    string CheckContents() const;
    int ReturnPageTally();
    int GetFrameAddress() const;
};

#endif
