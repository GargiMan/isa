/**
 * @file dns.h
 * @author Marek Gergel (xgerge01)
 * @brief declaration of functions and variables for dns resolver
 * @version 0.1
 * @date 2023-10-07
 */

#ifndef DNS_H
#define DNS_H

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <cstdbool>
#include <csignal>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "error.h"

const int MAX_TRANSFER_FAILS = 10;

class DNSHeader {
public:
    DNSHeader(bool recursion, bool inverse) {
        id = 0;
        flags = 0;
        qdcount = 1;
        ancount = 0;
        nscount = 0;
        arcount = 0;

        flags |= recursion ? RD : 0;
        flags |= inverse ? OP_INVERSE : 0;
    }

private:
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;

    enum FLAGS {
        QR_RESPONSE = 0x0080,
        OP_INVERSE = 0x0008,
        OP_STATUS = 0x0010,
        AA = 0x0004,
        TC = 0x0002,
        RD = 0x0001,
        RA = 0x1000,
    };
};

class DNSQuestion {
public:
    DNSQuestion(const std::string& address, const std::string& requestType) 
    : address(address), requestType(requestType) {};

private:
    std::string address;
    std::string requestType;
};

class DNSPacket {
public:
    DNSPacket(const DNSHeader& header, const DNSQuestion& question) 
    : header(header), question(question) {};

    std::vector<uint8_t> getBytes() const {
    std::vector<uint8_t> bytes;

    // Serialize the DNSHeader
    const uint8_t* headerData = reinterpret_cast<const uint8_t*>(&header);
    bytes.insert(bytes.end(), headerData, headerData + sizeof(DNSHeader));

    // Serialize the DNSQuestion
    const uint8_t* questionData = reinterpret_cast<const uint8_t*>(&question);
    bytes.insert(bytes.end(), questionData, questionData + sizeof(DNSQuestion));

    return bytes;
    }

private:
    DNSHeader header;
    DNSQuestion question;
};

void dns_init(const std::string& host, int port);
DNSPacket dns_send_packet(const DNSPacket& packet);
void dns_print_packet(const DNSPacket& packet);
void dns_close();


#endif // DNS_H
