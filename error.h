/**
 * @file error.h
 * @author Marek Gergel (xgerge01)
 * @brief declaration of functions and variables for error handling
 * @version 0.1
 * @date 2023-09-28
 */

#ifndef ERROR_H
#define ERROR_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

typedef enum errorCodes
{
    argumentError = 1,
    inputError = 2,
    socketError = 3,
    transferError = 4,
    signalError = 5
} errorCodes_t;

void error_exit(errorCodes_t errcode, char *msg, ...);
void warning_print(char *msg, ...);

#endif // ERROR_H
