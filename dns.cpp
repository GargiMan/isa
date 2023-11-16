/**
 * @file dns.cpp
 * @author Marek Gergel (xgerge01)
 * @brief definition of functions and variables for dns resolver
 * @version 0.1
 * @date 2023-10-07
 */

#include "dns.h"

using namespace std;

int socket_fd = -1;
struct addrinfo hints{}, *servinfo, *p;

/**
 * @brief Signal handler
 * @param signal received signal
 */
void sig_handler(const int signal) {
    switch (signal) {
        case SIGINT:
            dns_close();
            exit(0);
        case SIGALRM:
            dns_close();
            error_exit(ErrorCodes::TimeoutError, "Response timeout "+to_string(MAX_RESPONSE_WAIT_SEC)+"s");
        default:
            break;
    }
}

void dns_init(const string& host, const uint16_t port) {

    hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // Datagram socket

    int status;
    if ((status = getaddrinfo(host.c_str(), to_string(port).c_str(), &hints, &servinfo)) != 0) {
        error_exit(ErrorCodes::SocketError, "Server - " + string(gai_strerror(status)));
    }

    for (p = servinfo; p != nullptr; p = p->ai_next) {
        // Socket creation failed, try the next address
        if ((socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        // Connection failed, close the socket and try the next address
        if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_fd);
            continue;
        }

        // Address is not IPv4 or IPv6, close the socket and try the next address
        if (p->ai_family != AF_INET && p->ai_family != AF_INET6) {
            close(socket_fd);
            continue;
        }

        // Connected successfully
        break;
    }

    // No address succeeded
    if (p == nullptr) {
        if (servinfo != nullptr) {
            freeaddrinfo(servinfo);
        }
        error_exit(ErrorCodes::SocketError, "Socket creation failed");
        return;
    }

    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        dns_close();
        error_exit(ErrorCodes::SignalError, "Signal handler for 'SIGINT' registration failed");
    }

    if (signal(SIGALRM, sig_handler) == SIG_ERR) {
        dns_close();
        error_exit(ErrorCodes::SignalError, "Signal handler for 'SIGALRM' registration failed");
    }
}

/**
 * @brief Close the socket
 */
void dns_close() {
    freeaddrinfo(servinfo);
    close(socket_fd);
}

DNSPacket dns_send(const DNSPacket& packet) {

    if (p == nullptr) {
        error_exit(ErrorCodes::SocketError, "Socket not initialized");
    }

    uint8_t response_packet[BUFFER_SIZE] = "";

    // Send request to server
    int send_fails = 0;
    while (sendto(socket_fd, packet.getBytes().get(), packet.getSize(), 0, p->ai_addr, p->ai_addrlen) == -1) {
        if (++send_fails >= MAX_TRANSFER_FAILS) {
            dns_close();
            error_exit(ErrorCodes::TransferError, "Packet send failed");
        }
    }

    alarm(MAX_RESPONSE_WAIT_SEC);

    // Receive response from server
    int recv_fails = 0;
    while (recvfrom(socket_fd, response_packet, BUFFER_SIZE, 0, p->ai_addr, &p->ai_addrlen) == -1) {
        if (++recv_fails >= MAX_TRANSFER_FAILS) {
            dns_close();
            error_exit(ErrorCodes::TransferError, "Packet receive failed");
        }
    }

    alarm(0);

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