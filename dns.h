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
//data packet max size 4 + 65464
const int BUFFER_SIZE = 65468;

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

    DNSHeader(const uint8_t* buffer) {
        memcpy(&id, buffer, sizeof(uint16_t));
        memcpy(&flags, buffer + sizeof(uint16_t), sizeof(uint16_t));
        memcpy(&qdcount, buffer + 2 * sizeof(uint16_t), sizeof(uint16_t));
        memcpy(&ancount, buffer + 3 * sizeof(uint16_t), sizeof(uint16_t));
        memcpy(&nscount, buffer + 4 * sizeof(uint16_t), sizeof(uint16_t));
        memcpy(&arcount, buffer + 5 * sizeof(uint16_t), sizeof(uint16_t));
        id = ntohs(id);
        flags = ntohs(flags);
        qdcount = ntohs(qdcount);
        ancount = ntohs(ancount);
        nscount = ntohs(nscount);
        arcount = ntohs(arcount);
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

    enum FLAGS {
        QR_RESPONSE = 0x8000,
        OP_STATUS = 0x1000,
        OP_INVERSE = 0x0800,
        AA = 0x0400,
        TC = 0x0200,
        RD = 0x0100,
        RA = 0x0080,
    };

private:
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
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
    }

    DNSQuestion(const std::string& address, uint16_t type)
    {
        int len = 0;
        this->address = "";
        for(size_t i = 0; i < address.length(); ++i) {
            if (len == 0) {
                len = address[i];
                if (i != 0) {
                    this->address += '.';
                }
            } else {
                this->address += address[i];
                len--;
            }
        }
        this->qtype = type;
        this->qclass = 0x0001;
    }

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
    : header(header), question(question) {
        memcpy(buffer, &header, sizeof(DNSHeader));
        size_t offset = sizeof(DNSHeader);
        memcpy(buffer + offset, question.getAddress(), strlen(question.getAddress()) + 1);
        offset += strlen(question.getAddress()) + 1;
        uint16_t qtype = question.getQtype();
        memcpy(buffer + offset, &qtype, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        uint16_t qclass = question.getQclass();
        memcpy(buffer + offset, &qclass, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        bufferSize = offset;
    }

    DNSPacket(const uint8_t* buffer, size_t size) 
    : header(buffer), question(reinterpret_cast<const char*>(buffer + sizeof(DNSHeader)), ntohs(*reinterpret_cast<const uint16_t*>(buffer + sizeof(DNSHeader) + strlen(reinterpret_cast<const char*>(buffer + sizeof(DNSHeader))) + 1))) {
        memcpy(this->buffer, buffer, size);
        bufferSize = size;
    }

    const uint8_t* getBytes() const 
    {
        return reinterpret_cast<const uint8_t*>(buffer);
    }

    size_t getSize() const 
    {
        return bufferSize;
    }

    const DNSHeader& getHeader() const 
    {
        return header;
    }

    const DNSQuestion& getQuestion() const 
    {
        return question;
    }

private:
    DNSHeader header;
    DNSQuestion question;

    size_t bufferSize = 0;
    uint8_t buffer[BUFFER_SIZE];
};

void dns_init(const std::string& host, int port);
DNSPacket dns_send_packet(const DNSPacket& packet);
void dns_print_packet(const DNSPacket& packet);
void dns_close();

#endif // DNS_H
