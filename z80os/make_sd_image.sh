#!/bin/sh

rm -f sdcard.img
mkfs.vfat -F 32 -n Z80OS -C sdcard.img 1024
#sudo mount -o loop sdcard.img mnt/
#sudo cp kernel.bin mnt/
#sudo umount mnt
mcopy -i sdcard.img kernel.bin ::kernel.bin
hexdump -C sdcard.img > data.txt
