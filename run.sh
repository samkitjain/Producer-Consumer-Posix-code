#!/bin/bash
for i in {1..100}
do
make all
./main
python3 compare.py
done
