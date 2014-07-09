#!/bin/bash
sleep 1;
while :
do
    chmod +x kaerturnserver;
    ./kaerturnserver 192.168.40.192:5766 192.168.40.192 kaer tcp://192.168.40.192:5555;
    sleep 1;
done
