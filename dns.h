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
#include <bit>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "error.h"

const int MAX_TRANSFER_FAILS = 10;

enum RR_TYPE {
    A = 0x0001,
    NS = 0x0002,
    CNAME = 0x0005,
    SOA = 0x0006,
    PTR = 0x000c,
    MX = 0x000f,
    TXT = 0x0010,
    AAAA = 0x001c,
    SRV = 0x0021,
    ANY = 0x00ff,
};

class DNSHeader {
public:
    DNSHeader(bool recursion, bool inverse) {
        id = htons((unsigned short)getpid());
        flags = htons((recursion ? RD : 0 ) | (inverse ? OP_INVERSE : 0));
        qdcount = htons(1);
        ancount = 0;
        nscount = 0;
        arcount = 0;
    }

    uint16_t getId() const {
        return id;
    }

    uint16_t getFlags() const {
        return flags;
    }

    uint16_t getQdcount() const {
        return qdcount;
    }

    uint16_t getAncount() const {
        return ancount;
    }

    uint16_t getNscount() const {
        return nscount;
    }

    uint16_t getArcount() const {
        return arcount;
    }

private:
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;

    enum FLAGS {
        QR_RESPONSE = 0x1000,
        OP_STATUS = 0x8000,
        OP_INVERSE = 0x0100,
        AA = 0x0200,
        TC = 0x0400,
        RD = 0x0800,
        RA = 0x0010,
    };
};

class DNSQuestion {
public:
    DNSQuestion(const std::string& address, RR_TYPE type) 
    {
	    int pos = 0;
	    int len = 0;
        this->address = "\1";
        for(size_t i = 0; i < address.length(); ++i) {
            if(address[i] == '.') {
                this->address[pos] = len;
                len = 0;
                pos = i + 1;
                this->address += '\0';
            } else {
                this->address += address[i];
                len++;
            }
            if (i == address.length() - 1) {
                this->address[pos] = len;
            }
	    }
        this->qtype = htons(type);
        this->qclass = htons(0x0001);
    };

    const char* getAddress() const {
        return this->address.c_str();
    }

    uint16_t getQtype() const {
        return qtype;
    }

    uint16_t getQclass() const {
        return qclass;
    }

private:
    std::string address;
    uint16_t qtype;
    uint16_t qclass;
};

class DNSPacket {
public:
    DNSPacket(const DNSHeader& header, const DNSQuestion& question) 
    : header(header), question(question) {};

    std::vector<uint8_t> getBytes() const 
    {
        std::vector<uint8_t> bytes;

        // Serialize the DNSHeader
        bytes.push_back(static_cast<uint8_t>(header.getId() & 0xFF));               // Least significant byte
        bytes.push_back(static_cast<uint8_t>((header.getId() >> 8) & 0xFF));        // Most significant byte
        bytes.push_back(static_cast<uint8_t>(header.getFlags() & 0xFF));            // Least significant byte
        bytes.push_back(static_cast<uint8_t>((header.getFlags() >> 8) & 0xFF));     // Most significant byte
        bytes.push_back(static_cast<uint8_t>(header.getQdcount() & 0xFF));          // Least significant byte
        bytes.push_back(static_cast<uint8_t>((header.getQdcount() >> 8) & 0xFF));   // Most significant byte
        bytes.push_back(static_cast<uint8_t>(header.getAncount() & 0xFF));          // Least significant byte
        bytes.push_back(static_cast<uint8_t>((header.getAncount() >> 8) & 0xFF));   // Most significant byte
        bytes.push_back(static_cast<uint8_t>(header.getNscount() & 0xFF));          // Least significant byte
        bytes.push_back(static_cast<uint8_t>((header.getNscount() >> 8) & 0xFF));   // Most significant byte
        bytes.push_back(static_cast<uint8_t>(header.getArcount() & 0xFF));          // Least significant byte
        bytes.push_back(static_cast<uint8_t>((header.getArcount() >> 8) & 0xFF));   // Most significant byte

        // Serialize the DNSQuestion
        const char* str = question.getAddress();
        for (size_t i = 0; str[i] != '\0'; ++i)
        bytes.push_back(static_cast<uint8_t>(str[i]));
        bytes.push_back('\0'); 
        bytes.push_back(static_cast<uint8_t>(question.getQtype() & 0xFF));          // Least significant byte
        bytes.push_back(static_cast<uint8_t>((question.getQtype() >> 8) & 0xFF));   // Most significant byte
        bytes.push_back(static_cast<uint8_t>(question.getQclass() & 0xFF));         // Least significant byte
        bytes.push_back(static_cast<uint8_t>((question.getQclass() >> 8) & 0xFF));  // Most significant byte

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
