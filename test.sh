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

# opts:   type=A/AAAA/CNAME , recurse