#!/usr/bin/env bash
for((j=1;j<=10;j++))
do
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

echo "start players"


chmod u+x *

echo "start playmates"
./all_in   127.0.0.1 6000 127.0.0.1 6001 1111 0</dev/null 1>/dev/null 2>/dev/null &
./call     127.0.0.1 6000 127.0.0.2 6002 2222 0</dev/null 1>/dev/null 2>/dev/null &
./check    127.0.0.1 6000 127.0.0.3 6003 3333 0</dev/null 1>/dev/null 2>/dev/null &
./fold     127.0.0.1 6000 127.0.0.4 6004 4444 0</dev/null 1>/dev/null 2>/dev/null &
./raise1   127.0.0.1 6000 127.0.0.5 6005 5555 0</dev/null 1>/dev/null 2>/dev/null &
./raise100 127.0.0.1 6000 127.0.0.6 6006 6666 0</dev/null 1>/dev/null 2>/dev/null &
./random   127.0.0.1 6000 127.0.0.7 6007 7777 0</dev/null 1>/dev/null 2>/dev/null &

echo "start your game"
./game 127.0.0.1 6000 127.0.0.8 6008 8888
#gdb ./game -ex "r 127.0.0.1 6000 127.0.0.8 6008 8888"

wait
done
