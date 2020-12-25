#!/bin/bash
./font2c -i neodgm.ttf -o bitmap10x10_eng.bin -W 14 -H 11 -E -v -n > output.txt

./rzip a bitmap10x10_eng.bin

