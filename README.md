## DNS Resolver

Author: Marek Gergel (xgerge01)  
Date created: 28.09.2023 

### Description:
DNS resolver is a simple C++ program that can resolve domain names to IP addresses and vice versa. All DNS requests are sent to specified DNS server and program prints response in human readable format.
Program supports types of DNS queries A, NS, CNAME, SOA, PTR, MX, TXT, AAAA and ANY.

### Compilation:
Program can be compiled using Makefile by running `make` or `make all` command that creates executable file dns with g++ compiler. Minimum required C++ standard is C++14. 

### Usage:
Program can be run with following arguments:

`dns [-r] [-x] [-6] [-t TYPE] -s SERVER [-p PORT] ADDRESS [ADDRESS...]`  
`dns --help`  

#### Options:
`-r` - recursive resolution  
`-x` - type of DNS query PTR (reverse lookup)  
`-6` - type of DNS query AAAA (IPv6 address)  
`-t TYPE` - type of DNS query TYPE (default A) (TYPE is case insensitive)  
`-s SERVER` - IP address or hostname of DNS server  
`-p PORT` - port of DNS server (default 53)  
`ADDRESS` - IP address or hostname to resolve  
`--help` - print message with program info and usage

### Testing:
Program can be tested using `make test` command.
It runs program with different arguments and compares output with output from dig utility.

### Extensions and limits:
Program has following extensions:
- program support DNS queries types A, NS, CNAME, SOA, PTR, MX, TXT, AAAA and ANY in -t option
- program can be run with multiple addresses of same type to resolve 
- program supports IPv6 server addresses 
- program prints warning and error messages if something goes wrong

Program has following limits:
- program can print only record data of types that can request (A, NS, CNAME, SOA, PTR, MX, TXT, AAAA), other types of record data are printed in raw format
- program arguments are parsed with string comparison, so combination of short options (e.g. -rx) is not supported

### Files included: 
main.cpp, dns.h, dns.cpp, error.h, error.cpp, Makefile, README.md, manual.pdf
