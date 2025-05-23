#ifndef UTILS_H
#define UTILS_H

#include <string>

/*
* COLOR UTILS
*/
enum ConsoleColor {
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
    INVERTED,
    RESET
};

/**
 * @brief Prints text to the terminal in the given color.
 *
 * @param text  The message to print.
 * @param color The color to print the text in.
 */
void printColor(const std::string& text, ConsoleColor color);

/**
 * @brief Prints emmulator's title banner.
 */
void printBanner();

/**
 * @brief Prints emmulator's title subtitle.
 */
void printSubtitle();

/**
 * @brief Prints the help message containing available commands.
 */
void printHelp();

/**
* @bref Prints the emulator's placeholder consoles
*/
void printPlaceHolderConsoles();

/**
 * @brief Clears the terminal screen and re-prints the banner.
 */
void clear();

#endif // UTILS_H
