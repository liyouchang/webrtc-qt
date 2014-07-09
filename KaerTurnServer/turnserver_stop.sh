#!/bin/bash
ID=`ps -ef|grep turnserver|grep -v stop|grep -v grep| awk '{print $2}'`
echo $ID
echo "----------------"
for id in $ID
do 
	kill $id
	echo "killed $id"
done
echo "---------------"
