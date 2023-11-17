## DNS Resolver

Author: Marek Gergel (xgerge01)  
Date created: 28.09.2023 

### Description:
DNS resolver is a simple program that can resolve domain names to IP addresses and vice versa.

### Compilation:
Program can be compiled using Makefile by running `make` command or manually using following command:  
`g++ -std=c++14 -Wall -Wextra -Werror -pedantic -o dns main.cpp dns.cpp error.cpp`
Minimum required C++ standard is C++14.

### Usage:
Program can be run with following arguments:

`dns [-r] [-x] [-6] -s SERVER [-p PORT] ADDRESS [ADDRESS...]`  
`dns --help`  

#### Options:
`-r` - recursive resolution  
`-x` - reverse resolution  
`-6` - IPv6 address  
`-s SERVER` - IP address or hostname of DNS server  
`-p PORT` - port of DNS server (default 53)  
`ADDRESS` - IP address or hostname to resolve  
`--help` - print help message

### Files included: 
main.cpp, dns.h, dns.cpp, error.h, error.cpp, Makefile, README.md, manual.pdf

