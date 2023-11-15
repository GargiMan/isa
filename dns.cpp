/**
 * @file dns.cpp
 * @author Marek Gergel (xgerge01)
 * @brief definition of functions and variables for dns resolver
 * @version 0.1
 * @date 2023-10-07
 */

#include "dns.h"

using namespace std;

int socket_fd;

struct sockaddr_in sa;

/**
 * @brief Signal handler
 * @param signal received signal
 */
void sig_handler(const int signal)
{
    if (signal == SIGINT)
    {
        dns_close();
        exit(0);
    }
}

void dns_init(const string& host, const uint16_t port) {

    struct hostent *server;
    if ((server = gethostbyname(host.c_str())) == nullptr)
    {
        error_exit(ErrorCodes::SocketError, "Host '" + host + "' not found");
    }

    memset(&sa, 0, sizeof(sa));
    memcpy(&sa.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

    if ((socket_fd = socket(sa.sin_family, SOCK_DGRAM, 0)) <= 0)
    {
        error_exit(ErrorCodes::SocketError, "Socket creation failed");
    }

    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        error_exit(ErrorCodes::SignalError, "Signal handler registration failed");
    }
}

/**
 * @brief Close the socket
 */
void dns_close() {
    close(socket_fd);
}

DNSPacket dns_send(const DNSPacket& packet) {

    socklen_t address_len = sizeof(sa);
    uint8_t response_packet[BUFFER_SIZE] = "";

    // Send request to server
    int send_fails = 0;
    while (sendto(socket_fd, packet.getBytes().get(), packet.getSize(), 0, reinterpret_cast<sockaddr *>(&sa), address_len) == -1)
    {
        if (++send_fails >= MAX_TRANSFER_FAILS)
        {
            dns_close();
            error_exit(ErrorCodes::TransferError, "Packet send failed");
        }
    }

    // Receive response from server
    int recv_fails = 0;
    while (recvfrom(socket_fd, response_packet, BUFFER_SIZE, 0, reinterpret_cast<sockaddr *>(&sa), &address_len) == -1)
    {
        if (++recv_fails >= MAX_TRANSFER_FAILS)
        {
            dns_close();
            error_exit(ErrorCodes::TransferError, "Packet receive failed");
        }
    }

    DNSPacket response = DNSPacket(response_packet);

    return response;
}

void dns_print(const DNSPacket& packet) {
    //find longest name
    size_t longest_name = packet.getHeader().getQdcount() > 0 ? packet.getQuestion().getNameDot().length() : 0;
    for (const auto &record : packet.getAnswers()) {
        if (record.getName().length() > longest_name) {
            longest_name = record.getName().length();
        }
    }
    for (const auto &record : packet.getAuthorities()) {
        if (record.getName().length() > longest_name) {
            longest_name = record.getName().length();
        }
    }
    for (const auto &record : packet.getAdditionals()) {
        if (record.getName().length() > longest_name) {
            longest_name = record.getName().length();
        }
    }

    //print packet
    cout << "Authoritative: " << (packet.getHeader().getFlags() & DNSHeader::FLAGS::AA ? "Yes" : "No") << ", ";
    cout << "Recursion: " << (packet.getHeader().getFlags() & DNSHeader::FLAGS::RA && packet.getHeader().getFlags() & DNSHeader::FLAGS::RD ? "Yes" : "No") << ", ";
    cout << "Truncated: " << (packet.getHeader().getFlags() & DNSHeader::FLAGS::TC ? "Yes" : "No") << endl;
    cout << "Question section (" << packet.getHeader().getQdcount() << ")" << endl;
    if (packet.getHeader().getQdcount() > 0) {
        cout << "  " << setw(static_cast<int>(longest_name) + 15) << left << packet.getQuestion().getNameDot()
             << setw(10) << left << packet.getQuestion().getClassString()
             << setw(10) << left << packet.getQuestion().getTypeString() << endl;
    }
    cout << "Answer section (" << packet.getHeader().getAncount() << ")" << endl;
    for (const auto &record : packet.getAnswers()) {
        cout << "  " << setw(static_cast<int>(longest_name) + 4) << left << record.getName()
             << setw(11) << left << record.getTtl()
             << setw(10) << left << record.getClass()
             << setw(10) << left << record.getType()
             << record.getRdata() << endl;
    }
    cout << "Authority section (" << packet.getHeader().getNscount() << ")" << endl;
    for (const auto &record : packet.getAuthorities()) {
        cout << "  " << setw(static_cast<int>(longest_name) + 4) << left << record.getName()
             << setw(11) << left << record.getTtl()
             << setw(10) << left << record.getClass()
             << setw(10) << left << record.getType()
             << record.getRdata() << endl;
    }
    cout << "Additional section (" << packet.getHeader().getArcount() << ")" << endl;
    for (const auto &record : packet.getAdditionals()) {
        cout << "  " << setw(static_cast<int>(longest_name) + 4) << left << record.getName()
             << setw(11) << left << record.getTtl()
             << setw(10) << left << record.getClass()
             << setw(10) << left << record.getType()
             << record.getRdata() << endl;
    }
    cout << endl;
}