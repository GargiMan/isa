/**
 * @file main.cpp
 * @author Marek Gergel (xgerge01)
 * @brief main program for dns resolver
 * @version 0.1
 * @date 2023-09-28
 */

#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include "error.h"
#include "dns.h"

// variables for dns resolver
std::string address;
std::string server;
RR_TYPE type = RR_TYPE::A;
bool recursion = false;
bool inverse = false;
int port = 53;

/**
 * @brief Prints help message
 */
void print_help()
{
    std::cout << "Usage: dns [-r] [-x] [-6] -s SERVER [-p PORT] ADDRESS" << std::endl;
    std::cout << "       dns --help" << std::endl;
    std::cout << "       Send DNS request with ADDRESS to SERVER and print response" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -r          required recursion (Recursion Desired = 1), otherwise without recursion" << std::endl;
    std::cout << "  -x          inverse request, instead of straight" << std::endl;
    std::cout << "  -6          request type AAAA instead of default type A" << std::endl;
    std::cout << "  -s SERVER   server host name or IP address, where to send request" << std::endl;
    std::cout << "  -p PORT     server port number, default 53" << std::endl;
    std::cout << "  --help      print this help and exit program" << std::endl;
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
        if (std::string(argv[i]) == "-s" && i < argc - 1)
        {
            server = argv[++i];
        }
        else if (std::string(argv[i]) == "-p" && i < argc - 1)
        {
            char *endptr;
            port = std::strtol(argv[++i], &endptr, 10);

            if (*endptr != '\0' || port < 1 || port > 65535)
            {
                error_exit(ErrorCodes::ArgumentError, "Invalid port, port must be integer in range (0 - 65535)");
            }
        }
        else if (std::string(argv[i]) == "-r")
        {
            recursion = true;
        }
        else if (std::string(argv[i]) == "-x")
        {
            inverse = true;
        }
        else if (std::string(argv[i]) == "-6")
        {
            type = RR_TYPE::AAAA;
        }
        else if (std::string(argv[i]) == "--help")
        {
            print_help();
            std::exit(0);
        }
        else
        {
            if (address.empty())
            {
                address = argv[i];
                continue;
            }

            error_exit(ErrorCodes::ArgumentError, "Unknown argument '" + std::string(argv[i]) + "', use '--help' to show usage");
        }
    }

    if (address.empty() || server.empty())
    {
        error_exit(ErrorCodes::ArgumentError, "Option '-s SERVER' and ADDRESS are required arguments");
    }
}

/**
 * @brief Runs dns resolver program with given arguments, then prints response from server to stdout
 */
void dns_resolver()
{
    dns_init(server, port);

    DNSPacket packet = DNSPacket(DNSHeader(recursion, inverse), DNSQuestion(address, type));

    packet = dns_send_packet(packet);

    dns_print_packet(packet);

    dns_close();
}

int main(int argc, char *argv[])
{
    parse_args(argc, argv);

    dns_resolver();

    return 0;
}