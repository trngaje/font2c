#!/bin/bash

#./font2c -i neodgm.ttf -o bitmap10x10_kor.bin -W 12 -H 12 -v > output.txt
./font2c -i neodgm.ttf -o bitmap10x10_kor.bin -W 11 -H 11 -v > output.txt

./rzip a bitmap10x10_kor.bin

