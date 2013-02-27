#!/bin/bash

bochs -q &

gdb --eval-command "file tomos.bin" --eval-command "target remote localhost:4321" --eval-command c
