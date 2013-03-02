#!/bin/bash

#bochs -q &
qemu-system-i386 -gdb tcp::4321,ipv4 -fda fd.img -hda disk.img &

gdb --eval-command "file tomos.bin" --eval-command "target remote localhost:4321" # --eval-command c
