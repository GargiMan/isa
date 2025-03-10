% DNS Resolver  
% Marek Gergel (xgerge01)
\newpage

# Theoretical part

## DNS

DNS (Domain Name System) is a hierarchical and decentralized naming system for computers, services, or other resources connected to the Internet or a private network. It associates various information with domain names assigned to each of the participating entities. Most prominently, it translates more readily memorized domain names to the numerical IP addresses needed for locating and identifying computer services and devices with the underlying network protocols. By providing a worldwide, distributed directory service, the Domain Name System has been an essential component of the functionality of the Internet since 1985.

## DNS resolver

DNS resolver is a server that implements DNS protocol. It is used to resolve domain names to IP addresses and vice versa. DNS resolver can be implemented as a standalone server or as a part of other server (e.g. web server). DNS resolver can be also implemented as a part of operating system (e.g. Windows, Linux, macOS).

## DNS query

DNS query is a request for information sent from DNS client to DNS server. DNS query contains domain name or IP address and type of query. DNS server responds to DNS query with DNS response that contains requested information.

## DNS response

DNS response is a response to DNS query sent from DNS server to DNS client. DNS response contains requested information about domain name or IP address. DNS response can contain multiple records with different types of information.

## DNS record

DNS record is a data structure that contains information about domain name or IP address. DNS record contains type of record, time to live, class, name and data. DNS record can be stored in DNS server or DNS client.

## DNS types

DNS types are types of DNS records. DNS types are used to specify type of information that is requested in DNS query or that is stored in DNS record. DNS types are used in DNS query and DNS response.

## DNS classes

DNS classes are classes of DNS records. DNS classes are used to specify class of information that is requested in DNS query or that is stored in DNS record. DNS classes are used in DNS query and DNS response.

## DNS client

DNS client is a client that implements DNS protocol. DNS client is used to req resolve domain names to IP addresses and vice versa. DNS client can be implemented as a standalone client or as a part of other client (e.g. web browser). DNS client can be also implemented as a part of operating system (e.g. Windows, Linux, macOS). DNS client is also called DNS resolver. 

## DNS protocol

DNS protocol is a protocol that is used to resolve domain names to IP addresses and vice versa. DNS protocol is used in DNS query and DNS response. DNS protocol is implemented in DNS server and DNS client.

## DNS packet

DNS packet is a packet that contains DNS query or DNS response. DNS packet is used to send DNS query or DNS response between DNS server and DNS client. DNS packet is used in DNS protocol.

## DNS message

DNS message is a message that contains DNS query or DNS response. DNS message is used to send DNS query or DNS response between DNS server and DNS client. DNS message is used in DNS protocol.

## DNS header

DNS header is a header that contains information about DNS message. DNS header is used to send DNS message between DNS server and DNS client. DNS header is used in DNS protocol.

## DNS name

DNS name is a name that is used to identify domain name or IP address. DNS name is used in DNS query and DNS response.

## DNS resource record

DNS resource record is a record that contains information about domain name or IP address. DNS resource record is used in DNS query and DNS response. 

## DNS resource record data

DNS resource record data is a data that contains information about domain name or IP address. DNS resource record data is used in DNS query and DNS response. DNS resource record data is used in DNS protocol.

## DNS resource record type

DNS resource record type is a type of DNS resource record. DNS resource record type is used to specify type of information that is requested in DNS query or that is stored in DNS resource record. DNS resource record type is used in DNS query and DNS response.

## DNS resource record class

DNS resource record class is a class of DNS resource record. DNS resource record class is used to specify class of information that is requested in DNS query or that is stored in DNS resource record. DNS resource record class is used in DNS query and DNS response.

## DNS resource record time to live

DNS resource record time to live is a time to live of DNS resource record. DNS resource record time to live is used to specify time to live of information that is requested in DNS query or that is stored in DNS resource record. DNS resource record time to live is used in DNS query and DNS response.

## DNS resource record name

DNS resource record name is a name of DNS resource record. DNS resource record name is used to specify name of information that is requested in DNS query or that is stored in DNS resource record. DNS resource record name is used in DNS query and DNS response.

# Implementation

## main.cpp

File main.cpp contains main function that is used to run program. It parses arguments and runs DNS resolver with specified arguments.

Arguments are parsed using string comparison and each argument is parsed separately. Program supports following arguments:

| Argument    | Description                                                         |
| ----------- | ------------------------------------------------------------------- |
| `-r`        | recursive resolution                                                |
| `-6`        | type of DNS query AAAA (IPv6 address)                               |
| `-x`        | type of DNS query PTR (reverse lookup)                              |
| `-t TYPE`   | type of DNS query TYPE (default A) (TYPE is case insensitive)       |
| `-s SERVER` | IP address or hostname of DNS server (default obtained from system) |
| `-p PORT`   | port of DNS server (default 53)                                     |
| `ADDRESS`   | IP address or hostname to resolve                                   |
| `--help`    | print message with program info and usage                           |

Program can be run with multiple addresses of same type to resolve.

## dns.h

File dns.h contains classes DNSHeader, DNSQuestion, DNSRecord and DNSPacket with their attributes and methods.
All data manipulation methods are implemented in this file. 
Header file also contains constants, enums and dns resolver functions that are used in program.

Program supports DNS queries types A, NS, CNAME, SOA, PTR, MX, TXT, AAAA and ANY.

## dns.cpp

File dns.cpp contains implementation of methods from dns.h file.
Functions to initialize communication with DNS server, to send DNS query and print DNS response are implemented in this file.

## error.h

File error.h contains error codes and functions to print error messages.

## error.cpp

File error.cpp contains implementation of methods from error.h file.
