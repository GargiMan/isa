/**
 * @file main.c
 * @author Marek Gergel (xgerge01)
 * @brief main program for dns resolver
 * @version 0.1
 * @date 2023-09-28
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "error.h"

// variables for dns resolver
char *adress;
char *server;
char *request_type = "A";
int recursion = 0;
int reverse = 0;
int port = 53;

/**
 * @brief Prints help message
 */
void print_help()
{
    printf("Usage: dns [-r] [-x] [-6] -s SERVER [-p PORT] ADRESS\n");
    printf("       dns --help\n");
    printf("       Send DNS request with ADRESS to SERVER and print response\n");
    printf("Options:\n");
    printf("  -r            required recursion (Recursion Desired = 1), otherwise without recursion     \n");
    printf("  -x            reverse request, instead of straight\n");
    printf("  -6            request type AAAA instead of default type A\n");
    printf("  -s SERVER     server host name or IP address, where to send request\n");
    printf("  -p PORT       server port number, default 53\n");     
    printf("  --help        print this help and exit program\n");
}

/**
 * @brief Parse command line arguments
 * @param argc argument count
 * @param argv argument values
 */
void parse_args(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-s") == 0 && i < argc - 1)
        {
            server = argv[++i];
        }
        else if (strcmp(argv[i], "-p") == 0 && i < argc - 1)
        {
            char *endptr;
            port = (int)strtol(argv[++i], &endptr, 10);

            if (*endptr != '\0' || port < 1 || port > 65535)
            {
                error_exit(argumentError, "Invalid port, port must be integer in range (0 - 65535)\n");
            }
        }
        else if (strcmp(argv[i], "-r") == 0)
        {
            recursion = 1;
        }
        else if (strcmp(argv[i], "-x") == 0)
        {
            reverse = 1;
        }
        else if (strcmp(argv[i], "-6") == 0)
        {
            request_type = "AAAA";
        }
        else if (strcmp(argv[i], "--help") == 0)
        {
            print_help();
            exit(0);
        }
        else
        {
            if (adress == NULL)
            {
                adress = argv[i];
                continue;
            }

            error_exit(argumentError, "Unknown argument '%s', use '--help' to show usage\n", argv[i]);
        }
    }

    if (adress == NULL || server == NULL)
    {
        error_exit(argumentError, "Option '-s SERVER' and ADRESS are required arguments\n");
    }
}

/**
 * @brief Runs dns resolver program with given arguments, then prints response from server to stdout
 */
void dns_resolver()
{

}

int main(int argc, char *argv[])
{
    parse_args(argc, argv);

    dns_resolver();

    return 0;
}