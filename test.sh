#! /usr/bin/env bash

valgrind --leak-check=yes ./ex3_lb &
disown
sleep 5

./ex3_server `cat server_port` &
disown
./ex3_server `cat server_port` &
disown
./ex3_server `cat server_port` &
disown

python run_test.py `cat http_port` | diff 1 -
python run_test.py `cat http_port` | diff 1 -
python run_test.py `cat http_port` | diff 1 -

python run_test.py `cat http_port` | diff 2 -
python run_test.py `cat http_port` | diff 2 -
python run_test.py `cat http_port` | diff 2 -

python run_test.py `cat http_port` | diff 3 -
python run_test.py `cat http_port` | diff 3 -
python run_test.py `cat http_port` | diff 3 -

rm server_port
rm http_port

pkill -x -u `whoami` "ex3_lb"
pkill -x -u `whoami` "ex3_server"
pkill -f -u `whoami` "python2.7"
pkill -f -u `whoami` "python"

pkill -9 -x -u `whoami` "ex3_lb"
pkill -9 -x -u `whoami` "ex3_server"
pkill -9 -f -u `whoami` "python2.7"
pkill -9 -f -u `whoami` "python"
