#!/bin/bash
sleep 1;
while :
do
    chmod +x kaerturnserver;
    ./kaerturnserver 0.0.0.0:5766 192.168.40.179 kaer tcp://192.168.40.179:5555;
    sleep 1;
done
