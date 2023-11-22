#!/bin/sh
export POSIXLY_CORRECT=yes
export LC_ALL=C

addresses="www.google.com www.github.com www.fit.vut.cz"
dns_addresses="kazi.fit.vutbr.cz 8.8.8.8"

for address in $addresses; do
    for dns_address in $dns_addresses; do
        echo "dig $address @$dns_address"
        dig "$address" @"$dns_address"
        echo "dns $address $dns_address"
        ./dns -r -s "$dns_address" "$address" 
    done
done

#Test:
#-r -6 -s 8.8.8.8 2607:f8b0:4003:c00::6a
#-r -s kazi.fit.vutbr.cz www.fit.vut.cz
#-s 192.168.1.1 video1.fit.vutbr.cz wis.fit.vutbr.cz
#-x -s kazi.fit.vutbr.cz 142.251.37.100
#-x -s kazi.fit.vutbr.cz 2001:67c:1220:809::93e5:917
#-s kazi.fit.vutbr.cz wis.fit.vut.cz
# testy na valid a invalid ipv4, ipv6 a hostname, servername