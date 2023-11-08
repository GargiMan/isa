/**
 * @file error.cpp
 * @author Marek Gergel (xgerge01)
 * @brief definition of functions and variables for error handling
 * @version 0.1
 * @date 2023-07-11
 */

#include "error.h"

/**
 * @brief Prints an error message, free used memory and exits the program.
 * @param errcode error exit code
 * @param msg message to print
 */
void error_exit(ErrorCodes errcode, const std::string &msg)
{
    std::cerr << "Error: " << msg << std::endl;
    std::exit(static_cast<int>(errcode));
}

/**
 * @brief Prints a warning message.
 * @param msg message to print
 */
void warning_print(const std::string &msg)
{
    std::cerr << "Warning: " << msg << std::endl;
}
