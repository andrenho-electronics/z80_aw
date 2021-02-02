#!/usr/bin/env python3

import time
import spidev

bus = 0
device = 0

spi = spidev.SpiDev()
spi.open(bus, device)

spi.max_speed_hz = 100000 # 500 kHz
spi.mode = 0

def sd_cmd(cmd, arg, crc):
    spi.writebytes([
        cmd | 0x40,
        (arg >> 24) & 0xff,
        (arg >> 16) & 0xff,
        (arg >> 8) & 0xff,
        arg & 0xff,
        crc | 0x1])

def sd_cmd_r1(cmd, arg=0, crc=0):
    spi.cshigh = False
    spi.writebytes([0xff])
    sd_cmd(cmd, arg, crc)
    while True:
        r = spi.readbytes(1)
        if r[0] == 0xff:
            #spi.writebytes([0xff])
            pass
        else:
            return r[0]

print('Initializing SD card...')
spi.cshigh = True
time.sleep(0.001)
spi.writebytes([0xff] * 10)

print('Setting card in SPI mode...')
r = sd_cmd_r1(0x0, 0, 0x94)  # GO_IDLE_STATE
if r == 1:
    print('Card is in idle state (0x1).')
else:
    raise Exception('Card not in idle state: ' + str(r))

print('Running initialization process...')
r = sd_cmd_r1(0x55)  # CMD_55
if r != 1:
    raise Exception('Error in CMD_55: ' + str(r))
