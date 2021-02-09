#!/bin/sh

rm -f sdcard.img
dd if=/dev/zero of=sdcard.img bs=512 count=131072
mkfs.vfat -F 16 -n Z80OS sdcard.img
#sudo mount -o loop sdcard.img mnt/
#sudo cp kernel.bin mnt/
#sudo umount mnt
mcopy -i sdcard.img kernel.bin ::kernel.bin
hexdump -C sdcard.img > data.txt
