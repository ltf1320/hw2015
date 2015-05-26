#!/usr/bin/env bash

echo "prepare for test.."
killall game >/dev/null 2>&1
killall gameserver >/dev/null 2>&1

echo "start server"
pushd . >/dev/null
cd server
for i in 1 2 3 4 5 6 7 8
do 
  export "PLAYER"$i"_IP"=127.0.0.$i
  export "PLAYER"$i"_PORT"=600$i
  export "PLAYER"$i"_ID"=$i$i$i$i
done
chmod u+x gameserver
./gameserver -gip 127.0.0.1 -seq replay -r 30 -d 1 -m 10000 -b 50 -t 2000 -h 500 0</dev/null 1>/dev/null 2>/dev/null  & 
popd >/dev/null

echo "start your program"
pushd . >/dev/null
cd target
for i in 1 2 3 4 5 6 7 8
do
chmod u+x game 
./game 127.0.0.1 6000 127.0.0.$i 600$i $i$i$i$i 0</dev/null &
done
popd >/dev/null


wait

