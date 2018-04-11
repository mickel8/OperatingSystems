#!/bin/bash

trap 'echo "Proces potomny - otrzymałem sygnał SIGINT"; exit 1' 2
trap 'echo "Proces potomny - otrzymałem sygnał SIGTSTP"; exit 1' 20
 
while [ 1 ]; do
    date
    sleep 1
done