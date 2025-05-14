#ifndef UTILS_H
#define UTILS_H

#include <string>

/*
* COLOR UTILS
*/
// Define color codes as an enum for clarity
enum ConsoleColor {
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    RESET
};

// Function to print in color
void printColor(const std::string& text, ConsoleColor color);

/*
*  PRINT BANNER
*/
void printBanner();

#endif // UTILS_H
