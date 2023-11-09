/**
 * @file dns.cpp
 * @author Marek Gergel (xgerge01)
 * @brief definition of functions and variables for dns resolver
 * @version 0.1
 * @date 2023-10-07
 */

#include "dns.h"

//temp
const int BUFFER_SIZE = 65468;

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

    //data packet max size 4 + 65464
    uint8_t response_packet[BUFFER_SIZE] = "";

    // Send request to server
    int send_fails = 0;
    std::vector<uint8_t> request_packet = packet.getBytes();
    while (sendto(socket_fd, request_packet.data(), request_packet.size(), 0, (struct sockaddr *)&address, address_len) == -1)
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
    while (false && recvfrom(socket_fd, response_packet, BUFFER_SIZE, 0, (struct sockaddr *)&address, &address_len) == -1)
    {
        if (++recv_fails >= MAX_TRANSFER_FAILS)
        {
            dns_close();
            error_exit(ErrorCodes::TransferError, "Receive failed " + std::to_string(recv_fails) + " times");
        }
        warning_print("Receive failed");
    }

    return packet;
}

void dns_print_packet(const DNSPacket& packet) {
    std::cout << packet.getBytes().data() << std::endl;
}