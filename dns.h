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
#include <memory>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "error.h"

using namespace std;

constexpr int MAX_TRANSFER_FAILS = 10;
//data packet max size 4 + 65464
constexpr int BUFFER_SIZE = 65468;

inline uint16_t test_value = 0x01;
inline bool is_little_endian = (*reinterpret_cast<uint8_t*>(&test_value)) == 0x01;
inline uint16_t htonse(const uint16_t value) {
    return is_little_endian ? htons(value) : value;
}
inline uint16_t ntohse(const uint16_t value) {
    return is_little_endian ? ntohs(value) : value;
}
inline uint32_t htonle(const uint32_t value) {
    return is_little_endian ? htonl(value) : value;
}
inline uint32_t ntohle(const uint32_t value) {
    return is_little_endian ? ntohl(value) : value;
}

inline string getNameToDot(const uint8_t* buffer) {
    string name;
    while (buffer[0] != 0) {
        name += string(reinterpret_cast<const char*>(buffer + 1), buffer[0]);
        buffer += buffer[0] + 1;
        if (buffer[0] != 0) {
            name += ".";
        }
    }
    return name;
}

inline string getNameToDns(string address) {
    int pos = 0;
    char len = 0;
    string name = "\1";
    for(size_t i = 0; i < address.length(); ++i) {
        if(address[i] == '.') {
            name[pos] = len;
            len = 0;
            pos = i + 1;
            name += '\0';
        } else {
            name += address[i];
            len++;
        }
        if (i == address.length() - 1) {
            name[pos] = len;
        }
    }

    return name;
}

class RR_TYPE {
public:
    enum Type : uint16_t {
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

    RR_TYPE() = delete;
    constexpr RR_TYPE(const Type type) : type(type) {}

    explicit operator uint16_t() const {
        return type;
    }

    constexpr bool operator==(const RR_TYPE a) const { return type == a.type; }
    constexpr bool operator!=(const RR_TYPE a) const { return type != a.type; }

    uint16_t value() const {
        return type;
    }

    string toString() const  {
        return typeToString(type);
    }

    static string typeToString(const uint16_t type) {
        switch (type) {
            case A:
                return "A";
            case NS:
                return "NS";
            case CNAME:
                return "CNAME";
            case SOA:
                return "SOA";
            case PTR:
                return "PTR";
            case MX:
                return "MX";
            case TXT:
                return "TXT";
            case AAAA:
                return "AAAA";
            case SRV:
                return "SRV";
            case ANY:
                return "ANY";
            default:
                return "UNKNOWN";
        }
    }

private:
    Type type;
};

class DNSHeader {
public:
    DNSHeader() = default;

    DNSHeader(const bool recursion, const bool inverse, const uint16_t qdcount) {
        this->id = static_cast<uint16_t>(getpid());
        this->flags = (recursion ? RD : 0) | (inverse ? OP_INVERSE : 0);
        this->qdcount = qdcount;
    }

    DNSHeader(const uint8_t* buffer) {
        memcpy(&id, buffer, sizeof(uint16_t));
        memcpy(&flags, buffer + sizeof(uint16_t), sizeof(uint16_t));
        memcpy(&qdcount, buffer + 2 * sizeof(uint16_t), sizeof(uint16_t));
        memcpy(&ancount, buffer + 3 * sizeof(uint16_t), sizeof(uint16_t));
        memcpy(&nscount, buffer + 4 * sizeof(uint16_t), sizeof(uint16_t));
        memcpy(&arcount, buffer + 5 * sizeof(uint16_t), sizeof(uint16_t));
        this->id = ntohse(id);
        this->flags = ntohse(flags);
        this->qdcount = ntohse(qdcount);
        this->ancount = ntohse(ancount);
        this->nscount = ntohse(nscount);
        this->arcount = ntohse(arcount);

        switch (flags & RCODE_MASK) {
            case 0:
                break;
            case 1:
                warning_print("Format error - The name server was unable to interpret the query.");
                break;
            case 2:
                warning_print("Server failure - The name server was unable to process this query due to a problem with the name server.");
                break;
            case 3:
                warning_print("Name Error - Meaningful only for responses from an authoritative name server, the domain name referenced in the query does not exist.");
                break;
            case 4:
                warning_print("Not Implemented - The name server does not support the requested kind of query.");
                break;
            case 5:
                warning_print("Refused - The name server refuses to perform the specified operation for policy reasons.");
                break;
            case 6:
                warning_print("YXDomain - Name Exists when it should not.");
                break;
            case 7:
                warning_print("YXRRSet - RR Set Exists when it should not.");
                break;
            case 8:
                warning_print("NotAuth - Server Not Authoritative for zone.");
                break;
            case 9:
                warning_print("NotZone - Name not contained in zone.");
                break;
            default:
                warning_print("Unknown error");
                break;
        }
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
        RCODE_MASK = 0x000f,
    };

private:
    uint16_t id = 0;
    uint16_t flags = 0;
    uint16_t qdcount = 0;
    uint16_t ancount = 0;
    uint16_t nscount = 0;
    uint16_t arcount = 0;
};

class DNSQuestion {
public:
    DNSQuestion() = default;

    DNSQuestion(const string& address, const RR_TYPE type)
    {
        this->name = address;
        this->type = type.value();
        this->class_ = 0x0001;
    }

    DNSQuestion(const uint8_t* buffer)
    {
        this->name = getNameToDot(buffer);
        this->type = ntohse(*reinterpret_cast<const uint16_t*>(buffer + this->name.length() + 2));
        this->class_ = ntohse(*reinterpret_cast<const uint16_t*>(buffer + this->name.length() + 4));
    }

    string getNameDot() const {
        return this->name;
    }

    string getNameDns() const {
        return getNameToDns(this->name);
    }

    uint16_t getType() const {
        return type;
    }

    string getTypeString() const {
        return RR_TYPE::typeToString(type);
    }

    uint16_t getClass() const {
        return class_;
    }

    string getClassString() const {
        switch (class_) {
            case 0x0001:
                return "IN";
            default:
                return "UNKNOWN";
        }
    }

private:
    string name;
    uint16_t type = 0;
    uint16_t class_ = 0;
};

class DNSRecord {
public:
    DNSRecord(const uint8_t* buffer, const uint8_t* packet) {
        size_t offset = 0;

        uint16_t name = 0;
        memcpy(&name, buffer, sizeof(uint16_t));
        if (ntohse(name) == 0x0000) {
            offset += sizeof(uint8_t);
            this->name = "@";   //root
        } else {
            offset += sizeof(uint16_t);
            this->name = getNameToDot(packet + (ntohse(name) & 0x00ff));
        }

        memcpy(&type, buffer + offset, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        this->type = ntohse(type);

        memcpy(&class_, buffer + offset, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        this->class_ = ntohse(class_);

        memcpy(&ttl, buffer + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        this->ttl = ntohle(ttl);

        memcpy(&rdlength, buffer + offset, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        this->rdlength = ntohse(rdlength);

        this->rdata = string(reinterpret_cast<const char*>(buffer + offset), rdlength);
    }

    string getName() const {
        return name;
    }

    string getType() const {
        return RR_TYPE::typeToString(type);
    }

    string getClass() const {
        switch (class_) {
            case 0x0001:
                return "IN";
            default:
                return "UNKNOWN";
        }
    }

    uint32_t getTtl() const {
        return ttl;
    }

    uint16_t getRdlength() const {
        return rdlength;
    }

    string getRdata() const {
        string result;
        size_t offset;
        switch (type) {
            case RR_TYPE::A:
                for (int i = 0; i < rdlength; i++) {
                    result += to_string((uint8_t)rdata[i]);
                    if (i != rdlength - 1) {
                        result += ".";
                    }
                }
                break;
            case RR_TYPE::AAAA:
                for (int i = 0; i < rdlength; i += 2) {
                    result += to_string(rdata[i]);
                    result += to_string(rdata[i + 1]);
                    if (i != rdlength - 2) {
                        result += ":";
                    }
                }
                break;
            case RR_TYPE::SOA:
                result += getNameToDot(reinterpret_cast<const uint8_t*>(rdata.c_str()));
                result += "\t";
                offset = strlen(rdata.c_str());
                result += getNameToDot(reinterpret_cast<const uint8_t*>(rdata.c_str()) + offset + 1);
                result += "\t";
                offset += strlen(rdata.c_str() + offset + 1) + 2;
                result += to_string(ntohle(*reinterpret_cast<const uint32_t*>(rdata.c_str() + offset)));
                result += "\t";
                offset += 4;
                result += to_string(ntohle(*reinterpret_cast<const uint32_t*>(rdata.c_str() + offset)));
                result += "\t";
                offset += 4;
                result += to_string(ntohle(*reinterpret_cast<const uint32_t*>(rdata.c_str() + offset)));
                result += "\t";
                offset += 4;
                result += to_string(ntohle(*reinterpret_cast<const uint32_t*>(rdata.c_str() + offset)));
                result += "\t";
                offset += 4;
                result += to_string(ntohle(*reinterpret_cast<const uint32_t*>(rdata.c_str() + offset)));
                break;
            case RR_TYPE::PTR: case RR_TYPE::NS: case RR_TYPE::CNAME:
                result += getNameToDot(reinterpret_cast<const uint8_t*>(rdata.c_str()));
                break;
            case RR_TYPE::MX:
                result += to_string(ntohse(*reinterpret_cast<const uint16_t*>(rdata.c_str())));
                result += "\t";
                result += getNameToDot(reinterpret_cast<const uint8_t*>(rdata.c_str()) + 2);
                break;
            default:
                return rdata;
        }
        return result;
    }

private:
    string name;
    uint16_t type = 0;
    uint16_t class_ = 0;
    uint32_t ttl = 0;
    uint16_t rdlength = 0;
    string rdata;
};

class DNSPacket {
public:
    DNSPacket(const DNSHeader& header, const vector<DNSQuestion>& questions)
    {
        this->header = header;
        this->questions.insert(this->questions.end(), questions.begin(), questions.end());
    }

    DNSPacket(const uint8_t* buffer)
    {
        this->header = DNSHeader(buffer);
        size_t offset = 6 * sizeof(uint16_t);
        for (int i = 0; i < header.getQdcount(); i++) {
            DNSQuestion question(buffer + offset);
            questions.push_back(question);
            offset += 2 * sizeof(uint16_t) + question.getNameDns().length() + 1;
        }
        for (int i = 0; i < header.getAncount(); i++) {
            DNSRecord answer(buffer + offset, buffer);
            answers.push_back(answer);
            offset += 3 * sizeof(uint16_t) + sizeof(uint32_t) + answer.getRdlength();
        }
        for (int i = 0; i < header.getNscount(); i++) {
            DNSRecord authority(buffer + offset, buffer);
            authorities.push_back(authority);
            offset += 3 * sizeof(uint16_t) + sizeof(uint32_t) + authority.getRdlength();
        }
        for (int i = 0; i < header.getArcount(); i++) {
            DNSRecord additional(buffer + offset, buffer);
            additionals.push_back(additional);
            offset += 3 * sizeof(uint16_t) + sizeof(uint32_t) + additional.getRdlength();
        }
    }

    unique_ptr<uint8_t[]> getBytes() const
    {
        unique_ptr<uint8_t[]> buffer(new uint8_t[getSize()]);
        //header
        const uint16_t id = htonse(header.getId());
        memcpy(buffer.get(), &id, sizeof(uint16_t));
        size_t offset = sizeof(uint16_t);
        const uint16_t flags = htonse(header.getFlags());
        memcpy(buffer.get() + offset, &flags, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        const uint16_t qdcount = htonse(header.getQdcount());
        memcpy(buffer.get() + offset, &qdcount, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        const uint16_t ancount = htonse(header.getAncount());
        memcpy(buffer.get() + offset, &ancount, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        const uint16_t nscount = htonse(header.getNscount());
        memcpy(buffer.get() + offset, &nscount, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        const uint16_t arcount = htonse(header.getArcount());
        memcpy(buffer.get() + offset, &arcount, sizeof(uint16_t));
        offset += sizeof(uint16_t);

        //questions
        for (const auto& question : questions) {
            memcpy(buffer.get() + offset, question.getNameDns().c_str(), question.getNameDns().length() + 1);
            offset += question.getNameDns().length() + 1;
            uint16_t qtype = htonse(question.getType());
            memcpy(buffer.get() + offset, &qtype, sizeof(uint16_t));
            offset += sizeof(uint16_t);
            uint16_t qclass = htonse(question.getClass());
            memcpy(buffer.get() + offset, &qclass, sizeof(uint16_t));
            offset += sizeof(uint16_t);
        }

        return buffer;
    }

    size_t getSize() const
    {
        //header
        size_t offset = 6 * sizeof(uint16_t);

        //questions
        for (const auto& question : questions) {
            offset += question.getNameDns().length() + 1 + 2 * sizeof(uint16_t);
        }

        return offset;
    }

    const DNSHeader& getHeader() const 
    {
        return header;
    }

    const vector<DNSQuestion>& getQuestions() const
    {
        return questions;
    }

    const vector<DNSRecord>& getAnswers() const
    {
        return answers;
    }

    const vector<DNSRecord>& getAuthorities() const
    {
        return authorities;
    }

    const vector<DNSRecord>& getAdditionals() const
    {
        return additionals;
    }

private:
    DNSHeader header;
    vector<DNSQuestion> questions;
    vector<DNSRecord> answers;
    vector<DNSRecord> authorities;
    vector<DNSRecord> additionals;
};

void dns_init(const string& host, uint16_t port);
DNSPacket dns_send_packet(const DNSPacket& packet);
void dns_print_packet(const DNSPacket& packet);
void dns_close();

#endif // DNS_H
