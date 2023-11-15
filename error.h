/**
 * @file error.h
 * @author Marek Gergel (xgerge01)
 * @brief declaration of functions and variables for error handling
 * @version 0.1
 * @date 2023-07-11
 */

#ifndef ERROR_H
#define ERROR_H

#include <iostream>
#include <string>

enum class ErrorCodes
{
    ArgumentError = 1,
    InputError = 2,
    SocketError = 3,
    TransferError = 4,
    SignalError = 5
};

void error_exit(ErrorCodes errcode, const std::string &msg);
void warning_print(const std::string &msg);

#endif // ERROR_H
