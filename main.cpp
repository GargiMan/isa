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
#include <algorithm>

#include "error.h"
#include "dns.h"

using namespace std;

// variables for dns resolver
vector<string> addresses;
string server;
RR_TYPE type = RR_TYPE::A;
bool recursion = false;
long port = 53;

bool got_type = false;
bool got_server = false;
bool got_port = false;
bool got_recursion = false;

/**
 * @brief Prints help message
 */
void print_help() {
    cout << "Usage: dns [-r] [-6 | -x | -t TYPE] [-s SERVER] [-p PORT] ADDRESS [ADDRESS...]" << endl;
    cout << "       dns --help" << endl;
    cout << "       Send DNS requests for all ADDRESS (IPv4) values to DNS server and print responses" << endl;
    cout << "Options:" << endl;
    cout << "  -r          recursion desired, otherwise without recursion" << endl;
    cout << "  -6          request type AAAA (IPv6) instead of default type A (IPv4)" << endl;
    cout << "  -x          request type PTR (domain) instead of default type A (IPv4)" << endl;
    cout << "  -t TYPE     request type TYPE instead of default type A" << endl;
    cout << "              TYPE can be one of: A, NS, CNAME, SOA, PTR, MX, TXT, AAAA, ANY" << endl;
    cout << "  -s SERVER   DNS server host name or IP address, where to send request" << endl;
    cout << "              default server is obtained from system configuration" << endl;
    cout << "  -p PORT     DNS server port number, default 53" << endl;
    cout << "  ADDRESS     IPv4/IPv6 address or domain depending on request type" << endl;
    cout << "  --help      print this help and exit program" << endl;
}

/**
 * @brief Parse command line arguments
 * @param argc argument count
 * @param argv argument values
 */
void parse_args(const int argc, const char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "--help") {
            print_help();
            exit(0);
        }
    }

    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "-s" && i < argc - 1) {
            if (got_server) {
                error_exit(ErrorCodes::ArgumentError, "Option '-s' cannot be used multiple times");
            }
            server = argv[++i];
            got_server = true;
        } else if (string(argv[i]) == "-p" && i < argc - 1) {
            if (got_port) {
                error_exit(ErrorCodes::ArgumentError, "Option '-p' cannot be used multiple times");
            }
            char *endptr;
            port = strtol(argv[++i], &endptr, 10);

            if (*endptr != '\0' || port < 0 || port > 65535) {
                error_exit(ErrorCodes::ArgumentError, "Invalid port, port must be integer in range (0 - 65535)");
            }
            got_port = true;
        } else if (string(argv[i]) == "-r") {
            if (got_recursion) {
                error_exit(ErrorCodes::ArgumentError, "Option '-r' cannot be used multiple times");
            }
            recursion = true;
            got_recursion = true;
        } else if (string(argv[i]) == "-x") {
            if (got_type) {
                error_exit(ErrorCodes::ArgumentError, "Option '-x' cannot be used with '-6' or '-t' or used multiple times");
            }
            type = RR_TYPE::PTR;
            got_type = true;
        } else if (string(argv[i]) == "-6") {
            if (got_type) {
                error_exit(ErrorCodes::ArgumentError, "Option '-6' cannot be used with '-x' or '-t' or used multiple times");
            }
            type = RR_TYPE::AAAA;
            got_type = true;
        } else if (string(argv[i]) == "-t" && i < argc - 1) {
            if (got_type) {
                error_exit(ErrorCodes::ArgumentError, "Option '-t' cannot be used with '-x' or '-6' or used multiple times");
            }
            i++;
            string type_arg = string(argv[i]);
            transform(type_arg.begin(), type_arg.end(), type_arg.begin(), ::toupper);
            if (type_arg == "A") {
                type = RR_TYPE::A;
            } else if (type_arg == "NS") {
                type = RR_TYPE::NS;
            } else if (type_arg == "CNAME") {
                type = RR_TYPE::CNAME;
            } else if (type_arg == "SOA") {
                type = RR_TYPE::SOA;
            } else if (type_arg == "PTR") {
                type = RR_TYPE::PTR;
            } else if (type_arg == "MX") {
                type = RR_TYPE::MX;
            } else if (type_arg == "TXT") {
                type = RR_TYPE::TXT;
            } else if (type_arg == "AAAA") {
                type = RR_TYPE::AAAA;
            } else if (type_arg == "ANY") {
                type = RR_TYPE::ANY;
            } else {
                error_exit(ErrorCodes::ArgumentError, "Invalid type, TYPE value must be one of: A, NS, CNAME, SOA, PTR, MX, TXT, AAAA, ANY");
            }
            got_type = true;
        } else {
            if (argv[i][0] == '-') {
                error_exit(ErrorCodes::ArgumentError, "Unknown option '" + string(argv[i]) + "', use '--help' for available options");
            }

            addresses.emplace_back(argv[i]);
        }
    }

    if (server.empty()) {
        server = dns_get_default_server();
        if (server.empty()) {
            error_exit(ErrorCodes::ArgumentError, "Failed to obtain system configured DNS server, use option '-s SERVER' to specify server manually");
        }
        cout << "Default DNS server: " << server << endl;
    }

    if (addresses.empty()) {
        error_exit(ErrorCodes::ArgumentError, "Argument 'ADDRESS' is required");
    }
}

/**
 * @brief Runs dns resolver program with given arguments, then prints response from server to stdout
 */
void dns_resolver() {
    dns_init(server, static_cast<uint16_t>(port));

    for (const auto& address : addresses) {
        DNSPacket packet = DNSPacket(DNSHeader(recursion), DNSQuestion(address, type));

        packet = dns_send(packet);

        dns_print(packet);
    }

    dns_close();
}

int main(const int argc, const char *argv[]) {
    parse_args(argc, argv);

    dns_resolver();

    return 0;
}