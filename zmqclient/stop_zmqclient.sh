ID=`ps |grep zmqclient|grep -v stop|grep -v grep| awk '{print $1}'`
echo $ID
echo "----------------"
for id in $ID
do 
	kill $id
	echo "killed $id"
done
echo "---------------"
