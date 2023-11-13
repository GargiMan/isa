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
void sig_handler(const int signal)
{
    if (signal == SIGINT)
    {
        dns_close();
        std::exit(0);
    }
}

void dns_init(const std::string& host, uint16_t port) {

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
    while (sendto(socket_fd, packet.getBytes().get(), packet.getSize(), 0, (struct sockaddr *)&address, address_len) == -1)
    {
        if (++send_fails >= MAX_TRANSFER_FAILS)
        {
            dns_close();
            error_exit(ErrorCodes::TransferError, "Packet send failed");
        }
    }

    // Receive response from server
    int recv_fails = 0;
    while (recvfrom(socket_fd, response_packet, BUFFER_SIZE, 0, (struct sockaddr *)&address, &address_len) == -1)
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

void dns_print_packet(const DNSPacket& packet) {
    std::cout << "Authoritative: " << (packet.getHeader().getFlags() & DNSHeader::FLAGS::AA ? "Yes" : "No") << ", ";
    std::cout << "Recursion: " << ((packet.getHeader().getFlags() & DNSHeader::FLAGS::RA) && ( packet.getHeader().getFlags() & DNSHeader::FLAGS::RD) ? "Yes" : "No") << ", ";
    std::cout << "Truncated: " << (packet.getHeader().getFlags() & DNSHeader::FLAGS::TC ? "Yes" : "No") << std::endl;
    std::cout << "Question section (" << packet.getHeader().getQdcount() << ")" << std::endl;
    for (const auto& record : packet.getQuestions()) {
        std::cout << "\t" << record.getNameDot() << " " << record.getTypeString() << " " << record.getClassString() << std::endl;
    }
    std::cout << "Answer section (" << packet.getHeader().getAncount() << ")" << std::endl;
    for (const auto& record : packet.getAnswers()) {
        std::cout << "\t" << record.getName() << " " << record.getType() << " " << record.getClass() << " " << record.getTtl() << " " << record.getRdata() << std::endl;
    }
    std::cout << "Authority section (" << packet.getHeader().getNscount() << ")" << std::endl;
    for (const auto& record : packet.getAuthorities()) {
        std::cout << "\t" << record.getName() << " " << record.getType() << " " << record.getClass() << " " << record.getTtl() << " " << record.getRdata() << std::endl;
    }
    std::cout << "Additional section (" << packet.getHeader().getArcount() << ")" << std::endl;
    for (const auto& record : packet.getAdditionals()) {
        std::cout << "\t" << record.getName() << " " << record.getType() << " " << record.getClass() << " " << record.getTtl() << " " << record.getRdata() << std::endl;
    }
}