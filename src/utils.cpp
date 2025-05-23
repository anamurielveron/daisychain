#include "utils.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>


/**
* COLOR UTILS
*/
std::string getColorCode(ConsoleColor color) {
    switch (color) {
    case RED:		return "\033[31m";
    case GREEN:		return "\033[92m";
    case YELLOW:	return "\033[93m";
    case BLUE:		return "\033[94m";
    case MAGENTA:	return "\033[95m";
    case CYAN:		return "\033[96m";
    case WHITE:		return "\033[37m";
	case INVERTED:	return "\033[07m";
    case RESET:		return "\033[0m";
    default:		return "\033[0m";
    }
}

void printColor(const std::string& text, ConsoleColor color) {
	std::cout << getColorCode(color) << text << getColorCode(RESET);
}

/**
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

	bannerFile.close();
}

/**
*  PRINT SUBTITLE
*/
void printSubtitle() {
	std::cout << "------------------------------------------------------------" << std::endl;
	printColor(" Daisychain", YELLOW);
	printColor(": A Command-Line Emulator\n", WHITE);
	printColor(" Developed by ", WHITE);
	printColor("Ana", MAGENTA);
	printColor(", ", WHITE);
	printColor("Kaci", BLUE);
	printColor(", ", WHITE);
	printColor("Kian", CYAN);
	printColor(", ", WHITE);
	printColor("and ", WHITE);
	printColor("Liam", GREEN);
	printColor(" for CSOPESY @ DLSU\n", WHITE);
	std::cout << "------------------------------------------------------------" << std::endl;
	std::cout << std::endl;
}

/**
*  PRINT HELP
*/
void printHelp() {
	printColor("Available commands:\n", YELLOW);
	printColor("initialize\n", GREEN);
	printColor("screen\n", GREEN);
	printColor("scheduler-test\n", GREEN);
	printColor("scheduler-stop\n", GREEN);
	printColor("report-util\n", GREEN);
	printColor("clear\n", GREEN);
	printColor("exit\n", GREEN);
}

/**
* PRINT SHORTCUT CONSOLES (Takes in the shortcut keys)
*/
void printShortcut(const std::string& key, const std::string& label) {
	const int width = 30;  // fixed width for each shortcut block
	std::ostringstream oss;

	// Set keys and label to its respective colors
	std::ostringstream keyStream;
	keyStream << getColorCode(INVERTED) << key << getColorCode(RESET);
	std::ostringstream labelStream;
	labelStream << getColorCode(RESET) << label;

	// Combines the keys and label together
	oss << std::left << std::setw(width) << (keyStream.str() + " " + labelStream.str());
	std::cout << oss.str();
}


/**
* PLACEHOLDER CONSOLES
*/
void printPlaceHolderConsoles() {
	std::vector<std::pair<std::string, std::string>> shortcuts = {
	   {"^G", "Get Help"}, {"^O", "WriteOut "}, {"^R", "Read File"}, {"^Y", "Prev Page"},
	   {"^K", "Cut Text"}, {"^C", "Cur Pos"},  {"^X", "Exit"}, {"^J", "Justify"},  
	   {"^W", "Where Is"},  {"^V", "Next Page"},{"^U", "UnCut Text"},{"^T", "To Spell"}
	};

	// Print the first row
	for (size_t i = 0; i < shortcuts.size(); ++i) {
		printShortcut(shortcuts[i].first, shortcuts[i].second);
		if ((i + 1) % 6 == 0) std::cout << std::endl;
	}
	std::cout << std::endl;
}

/**
*  CLEAR SCREEN
*/
void clear() {
#ifdef _WIN32
	system("cls");
#else
	system("clear");
#endif
	printBanner();
}