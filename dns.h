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
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <csignal>
#include <memory>

#include "error.h"

#if defined(_WIN32) || defined(_WIN64) // windows

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

#define close(a) (void)closesocket(a)
#define socklen_t int

#else // unix

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fstream>

#endif // _WIN32 || _WIN64

using namespace std;

constexpr int MAX_TRANSFER_FAILS = 10;
constexpr int MAX_RESPONSE_WAIT_SEC = 10;
// according to RFC 1035, the maximum size of a UDP datagram is 512 bytes, but some DNS servers can send larger responses
constexpr int BUFFER_SIZE = 4096;

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

inline bool is_compressed(const uint8_t byte) {
    return (byte & 0xc0) == 0xc0;
}
inline uint16_t get_compressed_offset(const uint8_t* buffer) {
    return (static_cast<uint16_t>(buffer[0] & 0x3f) << 8) | buffer[1];
}

inline string getNameToDot(const uint8_t* buffer) {
    string name;

    while (buffer[0] != 0) {
        name += string(reinterpret_cast<const char*>(buffer + 1), buffer[0]);
        buffer += buffer[0] + 1;
        if (buffer[0] != 0) {
            name += ".";
            //pointer to another name
            if (is_compressed(buffer[0])) { 
                name += static_cast<char>(buffer[0]);
                name += static_cast<char>(buffer[1]);
                break;
            }
        }
    }

    return name;
}

inline string getNameToDotRef(const uint8_t* buffer, const uint8_t* packet) {
    string name = getNameToDot(buffer);

    if (is_compressed(name[name.length() - 2])) {
        const uint16_t name_offset = get_compressed_offset(reinterpret_cast<const uint8_t *>(name.c_str()) + name.length() - 2);
        name = name.substr(0, name.length() - 2);
        name += getNameToDot(packet + name_offset);
    }

    return name;
}

inline size_t getNameToDotRefLength(const uint8_t* buffer) {
    string name = getNameToDot(buffer);

    if (is_compressed(name[name.length() - 2])) {
        return name.length();
    }

    return name.length() + 2;
}

inline string getNameToDns(const string& address) {
    if (address.empty()) {
        return {'\0'};
    }

    size_t pos = 0;
    char len = 0;
    string name = "\1";

    for(size_t i = 0; i < address.length(); ++i) {
        if(address[i] == '.') {
            name[pos] = len;
            len = 0;
            pos = i + 1;
            if (name[name.length() - 1] != '\0') {
                name += '\0';
            }
        } else {
            name += address[i];
            len++;
        }

        if (i == address.length() - 1) {
            name[pos] = len;
        }
    }

    if (name[name.length() - 1] != '\0') {
        name += '\0';
    }

    return name;
}

inline string getInverseName(const string& address) {
    // Convert address from text to binary IPv4 address to reversed ARPA order
    in_addr ipv4{};
    if (inet_pton(AF_INET, address.c_str(), &ipv4) == 1) {
        char reverseArpa[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &ipv4, reverseArpa, sizeof(reverseArpa)) == nullptr) {
            warning_print("Error converting address back to ASCII characters");
            return "in-addr.arpa";
        }

        // Reverse the order of the octets
        string name;
        for (int i = 3; i >= 0; --i) {
            name += to_string((ipv4.s_addr >> (i * 8)) & 0xFF);
            name += ".";
        }

        return name+"in-addr.arpa";
    }

    // Convert address from text to binary IPv6 address to reversed ARPA order
    in6_addr ipv6{};
    if (inet_pton(AF_INET6, address.c_str(), &ipv6) == 1) {
        char reverseArpa[INET6_ADDRSTRLEN];
        if (inet_ntop(AF_INET6, &ipv6, reverseArpa, sizeof(reverseArpa)) == nullptr) {
            warning_print("Error converting address back to ASCII characters");
            return "ip6.arpa";
        }

        // 48 offset for ASCII 0-9, 87 offset for ASCII a-f
        // Reverse the order of the octets
        string name;
        for (int i = 15; i >= 0; --i) {
            name += static_cast<char>((ipv6.s6_addr[i] & 0xF) + ((ipv6.s6_addr[i] & 0xF) > 9 ? 87 : 48));
            name += ".";
            name += static_cast<char>((ipv6.s6_addr[i] >> 4 & 0xF) + ((ipv6.s6_addr[i] >> 4 & 0xF) > 9 ? 87 : 48));
            name += ".";
        }

        return name+"ip6.arpa";
    }

    warning_print("Address '"+address+"' is not valid IPv4 or IPv6 address");
    return ".";
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
        ANY = 0x00ff,
    };

    RR_TYPE() = delete;
    constexpr RR_TYPE(const Type type) : type(type) {}

    explicit operator uint16_t() const { return type; }
    explicit operator string() const { return typeToString(type); }
    constexpr bool operator==(const RR_TYPE a) const { return type == a.type; }
    constexpr bool operator!=(const RR_TYPE a) const { return type != a.type; }

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

    DNSHeader(const bool recursion) {
        this->id = static_cast<uint16_t>(getpid());
        this->flags = recursion ? RD : 0;
        this->qdcount = 1;
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

        if (this->id != static_cast<uint16_t>(getpid())) {
            warning_print("ID of response packet does not match ID of request packet");
        }

        if (!(flags & QR_RESPONSE)) {
            warning_print("Request packet received");
        }

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
                warning_print("Name error - The domain name referenced in the query does not exist.");
                break;
            case 4:
                warning_print("Not implemented - The name server does not support the requested kind of query.");
                break;
            case 5:
                warning_print("Refused - The name server refuses to perform the specified operation for policy reasons.");
                break;
            case 6:
                warning_print("YXDomain - Name exists when it should not.");
                break;
            case 7:
                warning_print("YXRRSet - RR set exists when it should not.");
                break;
            case 8:
                warning_print("NotAuth - Server not authoritative for zone.");
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

    DNSQuestion(const string& address, const RR_TYPE type) :
        name(type == RR_TYPE::Type::PTR ? getInverseName(address) : address),
        type(type),
        class_(0x0001) {}

    DNSQuestion(const uint8_t* buffer) :
        name(getNameToDot(buffer)),
        type(ntohse(*reinterpret_cast<const uint16_t*>(buffer + name.length() + (name.empty() ? 1 : 2)))),
        class_(ntohse(*reinterpret_cast<const uint16_t*>(buffer + name.length() + (name.empty() ? 3 : 4)))) {}

    string getNameDot() const {
        return this->name[this->name.length() - 1] == '.' ? this->name : this->name + ".";
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
            case 0x0002:
                return "CS";
            case 0x0003:
                return "CH";
            case 0x0004:
                return "HS";
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
    DNSRecord() = default;

    DNSRecord(const uint8_t* buffer, const uint8_t* packet) {
        this->packet = packet;
        size_t offset = 0;

        if (buffer[0] == 0x00) {
            this->name = "";
            offset += sizeof(uint8_t);
        } else if (is_compressed(buffer[0])) {
            this->name = getNameToDotRef(packet + get_compressed_offset(buffer), this->packet);
            offset += sizeof(uint16_t);
        } else {
            this->name = getNameToDot(buffer);
            offset += this->name.length() + 2;
        }

        memcpy(&type, buffer + offset, sizeof(uint16_t));
        this->type = ntohse(type);
        offset += sizeof(uint16_t);

        memcpy(&class_, buffer + offset, sizeof(uint16_t));
        this->class_ = ntohse(class_);
        offset += sizeof(uint16_t);

        memcpy(&ttl, buffer + offset, sizeof(uint32_t));
        this->ttl = ntohle(ttl);
        offset += sizeof(uint32_t);

        memcpy(&rdlength, buffer + offset, sizeof(uint16_t));
        this->rdlength = ntohse(rdlength);
        offset += sizeof(uint16_t);

        this->rdata = string(reinterpret_cast<const char*>(buffer + offset), rdlength);
        offset += rdlength;

        this->recordLength = offset;
    }

    size_t getRecordLength() const {
        return recordLength;
    }

    string getName() const {
        return name + ".";
    }

    string getType() const {
        return RR_TYPE::typeToString(type);
    }

    string getClass() const {
        switch (class_) {
            case 0x0001:
                return "IN";
            case 0x0002:
                return "CS";
            case 0x0003:
                return "CH";
            case 0x0004:
                return "HS";
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
        in6_addr ipv6{};
        string result;
        size_t offset;
        switch (type) {
            case RR_TYPE::A:
                if (rdlength != 4) {
                    warning_print("A record has invalid length");
                    return rdata;
                }
                for (int i = 0; i < rdlength; i++) {
                    // Convert each octet to ASCII character
                    result += to_string(static_cast<uint8_t>(rdata[i]));
                    if (i != rdlength - 1) {
                        result += ".";
                    }
                }
                break;
            case RR_TYPE::AAAA:
                if (rdlength != 16) {
                    warning_print("AAAA record has invalid length");
                    return rdata;
                }
                for (int i = 0; i < rdlength; i += 2) {
                    // 48 offset for ASCII 0-9, 87 offset for ASCII a-f
                    // Convert each octet to ASCII characters
                    result += static_cast<char>((rdata[i] >> 4 & 0xF) + ((rdata[i] >> 4 & 0xF) > 9 ? 87 : 48));
                    result += static_cast<char>((rdata[i] & 0xF) + ((rdata[i] & 0xF) > 9 ? 87 : 48));
                    result += static_cast<char>((rdata[i+1] >> 4 & 0xF) + ((rdata[i+1] >> 4 & 0xF) > 9 ? 87 : 48));
                    result += static_cast<char>((rdata[i+1] & 0xF) + ((rdata[i+1] & 0xF) > 9 ? 87 : 48));
                    if (i != rdlength - 2) {
                        result += ":";
                    }
                }
                // Convert expanded IPv6 address to shortened form
                if (inet_pton(AF_INET6, result.c_str(), &ipv6) == 1) {
                    char address[INET6_ADDRSTRLEN];
                    if (inet_ntop(AF_INET6, &ipv6, address, sizeof(address)) == nullptr) {
                        break;
                    }
                    result = string(address);
                }
                break;
            case RR_TYPE::SOA:
                result += getNameToDotRef(reinterpret_cast<const uint8_t*>(rdata.c_str()), this->packet);
                result += ". ";
                offset = getNameToDotRefLength(reinterpret_cast<const uint8_t*>(rdata.c_str()));
                result += getNameToDotRef(reinterpret_cast<const uint8_t*>(rdata.c_str()) + offset, this->packet);
                result += ". ";
                offset += getNameToDotRefLength(reinterpret_cast<const uint8_t*>(rdata.c_str()) + offset);
                result += to_string(ntohle(*reinterpret_cast<const uint32_t*>(rdata.c_str() + offset)));
                result += " ";
                offset += 4;
                result += to_string(ntohle(*reinterpret_cast<const uint32_t*>(rdata.c_str() + offset)));
                result += " ";
                offset += 4;
                result += to_string(ntohle(*reinterpret_cast<const uint32_t*>(rdata.c_str() + offset)));
                result += " ";
                offset += 4;
                result += to_string(ntohle(*reinterpret_cast<const uint32_t*>(rdata.c_str() + offset)));
                result += " ";
                offset += 4;
                result += to_string(ntohle(*reinterpret_cast<const uint32_t*>(rdata.c_str() + offset)));
                break;
            case RR_TYPE::PTR: case RR_TYPE::NS: case RR_TYPE::CNAME:
                result += getNameToDotRef(reinterpret_cast<const uint8_t*>(rdata.c_str()), this->packet);
                result += ".";
                break;
            case RR_TYPE::MX:
                result += to_string(ntohse(*reinterpret_cast<const uint16_t*>(rdata.c_str())));
                result += " ";
                result += getNameToDotRef(reinterpret_cast<const uint8_t*>(rdata.c_str()) + 2, this->packet);
                result += ".";
                break;
            case RR_TYPE::TXT:
                result += "\"";
                result += rdata.substr(1, rdata[0]);
                result += "\"";
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

    size_t recordLength = 0;
    const uint8_t* packet = nullptr;
};

class DNSPacket {
public:
    DNSPacket() = default;

    DNSPacket(const DNSHeader& header, const DNSQuestion& question) {
        this->header = header;
        this->question = question;
    }

    DNSPacket(const uint8_t* buffer) {
        this->header = DNSHeader(buffer);
        size_t offset = 6 * sizeof(uint16_t);
        this->question = DNSQuestion(buffer + offset);
        offset += 2 * sizeof(uint16_t) + question.getNameDns().length();
        for (int i = 0; i < header.getAncount(); i++) {
            DNSRecord answer(buffer + offset, buffer);
            answers.push_back(answer);
            offset += answer.getRecordLength();
        }
        for (int i = 0; i < header.getNscount(); i++) {
            DNSRecord authority(buffer + offset, buffer);
            authorities.push_back(authority);
            offset += authority.getRecordLength();
        }
        for (int i = 0; i < header.getArcount(); i++) {
            DNSRecord additional(buffer + offset, buffer);
            additionals.push_back(additional);
            offset += additional.getRecordLength();
        }
    }

    unique_ptr<uint8_t[]> getBytes() const {
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

        //question
        memcpy(buffer.get() + offset, question.getNameDns().c_str(), question.getNameDns().length());
        offset += question.getNameDns().length();
        uint16_t qtype = htonse(question.getType());
        memcpy(buffer.get() + offset, &qtype, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        uint16_t qclass = htonse(question.getClass());
        memcpy(buffer.get() + offset, &qclass, sizeof(uint16_t));

        return buffer;
    }

    size_t getSize() const {
        return 8 * sizeof(uint16_t) + question.getNameDns().length();
    }

    const DNSHeader& getHeader() const {
        return header;
    }

    const DNSQuestion& getQuestion() const {
        return question;
    }

    const vector<DNSRecord>& getAnswers() const {
        return answers;
    }

    const vector<DNSRecord>& getAuthorities() const {
        return authorities;
    }

    const vector<DNSRecord>& getAdditionals() const {
        return additionals;
    }

private:
    DNSHeader header;
    DNSQuestion question;
    vector<DNSRecord> answers;
    vector<DNSRecord> authorities;
    vector<DNSRecord> additionals;
};

void dns_init(const string& host, uint16_t port);
DNSPacket dns_send(const DNSPacket& packet);
void dns_print(const DNSPacket& packet);
void dns_close();

string dns_get_default_server();

#endif // DNS_H
