# DNS Proxy Server

This is a test task which is a simple DNS proxy server. </br>
This app reads configuration from a JSON file, processes DNS queries </br>
and can block certain domains.

#### How to run:

You can use the provided Makefile to build and run it:

    make

To clean up the output files:

    make clean

#### How to test:

    add or change str "nameserver 127.0.0.1"
    - sudo vim /etc/resolv.conf

    stop systemd-resolved
    - sudo systemctl stop systemd-resolved

    an answer this command must be empty(53 port should be free):
    - sudo lsof -i :53


#### Dependencies

    json-c: for working with JSON files.
    resolv: for extract domain name from DNS request.
