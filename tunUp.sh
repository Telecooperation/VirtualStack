#!/bin/bash
sudo ip link set tun0 up
sudo ip addr add 10.0.0.1/24 dev tun0
#sudo ip route add 192.168.178.37 via 10.0.0.1 dev tun0
#echo -n "1" > /dev/udp/10.0.0.1/8000