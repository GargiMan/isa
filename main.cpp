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

using namespace std;

// variables for dns resolver
vector<string> addresses;
string server;
RR_TYPE type = RR_TYPE::A;
bool recursion = false;
long port = 53;

/**
 * @brief Prints help message
 */
void print_help()
{
    cout << "Usage: dns [-r] [-x] [-6] -s SERVER [-p PORT] ADDRESS [ADDRESS...]" << endl;
    cout << "       dns --help" << endl;
    cout << "       Send DNS requests for all IPv4 ADDRESS values to SERVER and print responses" << endl;
    cout << "Options:" << endl;
    cout << "  -r          recursion desired, otherwise without recursion" << endl;
    cout << "  -x          request type PTR (domain) instead of default type A (IPv4)" << endl;
    cout << "  -6          request type AAAA (IPv6) instead of default type A (IPv4)" << endl;
    cout << "  -s SERVER   server host name or IP address, where to send request" << endl;
    cout << "  -p PORT     server port number, default 53" << endl;
    cout << "  ADDRESS     IPv4/IPv6 address or domain depending on request type" << endl;
    cout << "  --help      print this help and exit program" << endl;
}

/**
 * @brief Parse command line arguments
 * @param argc argument count
 * @param argv argument values
 */
void parse_args(const int argc, const char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if (string(argv[i]) == "-s" && i < argc - 1)
        {
            server = argv[++i];
        }
        else if (string(argv[i]) == "-p" && i < argc - 1)
        {
            char *endptr;
            port = strtol(argv[++i], &endptr, 10);

            if (*endptr != '\0' || port < 0 || port > 65535)
            {
                error_exit(ErrorCodes::ArgumentError, "Invalid port, port must be integer in range (0 - 65535)");
            }
        }
        else if (string(argv[i]) == "-r")
        {
            recursion = true;
        }
        else if (string(argv[i]) == "-x")
        {
            if (type == RR_TYPE::AAAA)
            {
                error_exit(ErrorCodes::ArgumentError, "Option '-x' cannot be used with '-6'");
            }
            type = RR_TYPE::PTR;
        }
        else if (string(argv[i]) == "-6")
        {
            if (type == RR_TYPE::PTR)
            {
                error_exit(ErrorCodes::ArgumentError, "Option '-6' cannot be used with '-x'");
            }
            type = RR_TYPE::AAAA;
        }
        else if (string(argv[i]) == "--help")
        {
            print_help();
            exit(0);
        }
        else
        {
            if (argv[i][0] == '-')
            {
                error_exit(ErrorCodes::ArgumentError, "Unknown option '" + string(argv[i]) + "'");
            }

            addresses.emplace_back(argv[i]);
        }
    }

    if (addresses.empty() || server.empty())
    {
        error_exit(ErrorCodes::ArgumentError, "Option '-s SERVER' and 'ADDRESS' are required arguments");
    }
}

/**
 * @brief Runs dns resolver program with given arguments, then prints response from server to stdout
 */
void dns_resolver()
{
    dns_init(server, static_cast<uint16_t>(port));

    for (size_t i = 0; i < addresses.size(); i++) {
        DNSPacket packet = DNSPacket(DNSHeader(recursion), DNSQuestion(addresses[i], type));

        packet = dns_send(packet);

        dns_print(packet);
    }

    dns_close();
}

int main(const int argc, const char *argv[])
{
    parse_args(argc, argv);

    dns_resolver();

    return 0;
}