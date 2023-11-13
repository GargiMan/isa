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
#include <regex>
#include "error.h"
#include "dns.h"

using namespace std;

// variables for dns resolver
vector<string> addresses;
string server;
RR_TYPE type = RR_TYPE::A;
bool recursion = false;
bool inverse = false;
long port = 53;

/**
 * @brief Prints help message
 */
void print_help()
{
    cout << "Usage: dns [-r] [-x] [-6] -s SERVER [-p PORT] ADDRESS [ADDRESS...]" << endl;
    cout << "       dns --help" << endl;
    cout << "       Send DNS request with all ADDRESS values to SERVER and print response" << endl;
    cout << "Options:" << endl;
    cout << "  -r          required recursion (Recursion Desired = 1), otherwise without recursion" << endl;
    cout << "  -x          inverse request, instead of straight" << endl;
    cout << "  -6          request type AAAA instead of default type A" << endl;
    cout << "  -s SERVER   server host name or IP address, where to send request" << endl;
    cout << "  -p PORT     server port number, default 53" << endl;
    cout << "  --help      print this help and exit program" << endl;
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
            inverse = true;
        }
        else if (string(argv[i]) == "-6")
        {
            if (inverse)
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
            regex ipv4_regex("^\\b(?:\\d{1,3}\\.){3}\\d{1,3}\\b$");
            regex ipv6_regex("^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))$");
            regex hostname_regex("^([a-zA-Z0-9_-]+\\.)*[a-zA-Z0-9_-]+\\b$");

            if (!regex_match(argv[i], ipv4_regex) && !regex_match(argv[i], ipv6_regex) && !regex_match(argv[i], hostname_regex)) {
                error_exit(ErrorCodes::ArgumentError, "Unknown argument '" + string(argv[i]) + "', use '--help' to show usage");
            }

            addresses.emplace_back(argv[i]);
        }
    }

    if (addresses.empty() || server.empty())
    {
        error_exit(ErrorCodes::ArgumentError, "Option '-s SERVER' and ADDRESS are required arguments");
    }
}

/**
 * @brief Runs dns resolver program with given arguments, then prints response from server to stdout
 */
void dns_resolver()
{
    dns_init(server, static_cast<uint16_t>(port));

    vector<DNSQuestion> questions;
    questions.reserve(addresses.size());
    for (const auto &address : addresses) {
        questions.emplace_back(address, type);
    }

    DNSPacket packet = DNSPacket(DNSHeader(recursion, inverse, questions.size()), questions);

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