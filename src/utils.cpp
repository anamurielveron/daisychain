#include "utils.h"
#include <iostream>
#include <fstream>

/*
* COLOR UTILS
*/
// ANSI color codes
std::string getColorCode(ConsoleColor color) {
    switch (color) {
    case RED:     return "\033[31m";
    case GREEN:   return "\033[92m";
    case YELLOW:  return "\033[93m";
    case BLUE:    return "\033[94m";
    case MAGENTA: return "\033[95m";
    case CYAN:    return "\033[96m";
    case WHITE:   return "\033[37m";
    case RESET:   return "\033[0m";
    default:      return "\033[0m";
    }
}

void printColor(const std::string& text, ConsoleColor color) {
	std::cout << getColorCode(color) << text << getColorCode(RESET);
}

/*
*  PRINT BANNER
*/
void printBanner() {
	std::ifstream bannerFile("data/banner.txt");
	if (!bannerFile.is_open()) {
		std::cerr << "Failed to load banner.txt. Skipping banner...\n";
		return;
	}

	std::string line;
	while (std::getline(bannerFile, line)) {
		printColor(line, YELLOW);
		std::cout << std::endl;
	}

	std::cout << std::endl;

	// Print the title and authors
	std::cout << "------------------------------------------------------------" << std::endl;
	printColor(" Daisychain", YELLOW);
	printColor(": A Command-Line Emulator", WHITE);
	std::cout << std::endl;
	printColor(" Developed by ", WHITE);
	printColor("Ana", MAGENTA);
	printColor(", ", WHITE);
	printColor("Kaci", BLUE);
	printColor(", ", WHITE);
	printColor("Kian", CYAN);
	printColor(", ", WHITE);
	printColor("and ", WHITE);
	printColor("Liam", GREEN);
	printColor(" for CSOPESY @ DLSU", WHITE);
	std::cout << std::endl;
	std::cout << "------------------------------------------------------------" << std::endl;

	bannerFile.close();
}