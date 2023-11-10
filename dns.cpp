/**
 * @file dns.cpp
 * @author Marek Gergel (xgerge01)
 * @brief definition of functions and variables for dns resolver
 * @version 0.1
 * @date 2023-10-07
 */

#include "dns.h"

int socket_fd;
struct sockaddr_in address;

/**
 * @brief Signal handler
 * @param signal received signal
 */
void sig_handler(int signal)
{
    if (signal == SIGINT)
    {
        dns_close();
        std::exit(0);
    }
}

void dns_init(const std::string& host, int port) {

    struct hostent *server;
    if ((server = gethostbyname(host.c_str())) == nullptr)
    {
        error_exit(ErrorCodes::SocketError, "Host '" + host + "' not found");
    }

    std::memset(&address, 0, sizeof(address));
    std::memcpy(&address.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) <= 0)
    {
        error_exit(ErrorCodes::SocketError, "Socket creation failed");
    }

    if (std::signal(SIGINT, sig_handler) == SIG_ERR)
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

DNSPacket dns_send_packet(const DNSPacket& packet) {

    socklen_t address_len = sizeof(address);
    uint8_t response_packet[BUFFER_SIZE] = "";

    // Send request to server
    int send_fails = 0;
    while (sendto(socket_fd, packet.getBytes(), packet.getSize(), 0, (struct sockaddr *)&address, address_len) == -1)
    {
        if (++send_fails >= MAX_TRANSFER_FAILS)
        {
            dns_close();
            error_exit(ErrorCodes::TransferError, "Send failed " + std::to_string(send_fails) + " times");
        }
        warning_print("Send failed\n");
    }

    // Receive response from server
    int recv_fails = 0;
    ssize_t response_size = 0;
    while ((response_size = recvfrom(socket_fd, response_packet, BUFFER_SIZE, 0, (struct sockaddr *)&address, &address_len)) == -1)
    {
        if (++recv_fails >= MAX_TRANSFER_FAILS)
        {
            dns_close();
            error_exit(ErrorCodes::TransferError, "Receive failed " + std::to_string(recv_fails) + " times");
        }
        warning_print("Receive failed");
    }

    DNSPacket response = DNSPacket(response_packet, response_size);

    return response;
}

void dns_print_packet(const DNSPacket& packet) {
    std::cout << "Authoritative: " << (packet.getHeader().getFlags() & DNSHeader::FLAGS::AA ? "yes" : "no") << std::endl;
    std::cout << "Recursion available: " << (packet.getHeader().getFlags() & DNSHeader::FLAGS::RA ? "yes" : "no") << std::endl;
    std::cout << "Recursion desired: " << (packet.getHeader().getFlags() & DNSHeader::FLAGS::RD ? "yes" : "no") << std::endl;
    std::cout << "Truncated: " << (packet.getHeader().getFlags() & DNSHeader::FLAGS::TC ? "yes" : "no") << std::endl;
    std::cout << "Question count: " << packet.getHeader().getQdcount() << std::endl;
    std::cout << packet.getQuestion().getAddress() << " " << packet.getQuestion().getQtype() << " " << packet.getQuestion().getQclass() << std::endl;
    std::cout << "Answer count: " << packet.getHeader().getAncount() << std::endl;
    std::cout << "Authority count: " << packet.getHeader().getNscount() << std::endl;
    std::cout << "Additional count: " << packet.getHeader().getArcount() << std::endl;
}