#!/bin/bash

./server

sleep 1

SERVER_IP="127.0.0.1"
SERVER_PORT="8080"
NUM_VISITORS="100"

echo "Запуск $NUM_VISITORS посетителей..."

for ((i=1; i<=$NUM_VISITORS; i++))
do
    ./client $SERVER_IP $SERVER_PORT $i &
done

echo "Все посетители запущены"
wait
echo "Все посетители завершили работу"