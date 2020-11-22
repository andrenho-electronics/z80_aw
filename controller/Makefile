PROJECT=z80_debugger
OBJECTS=main.o serial.o repl.o
AVRDUDE_FLAGS=-p atmega16 -C ./avrdude_gpio.conf -c pi_1 -b 300
#FUSES=-U lfuse:w:0xe4:m -U hfuse:w:0xd9:m   # internal 8 Mhz
FUSES=-U lfuse:w:0xff:m -U hfuse:w:0xd9:m    # external crystal
MCU=atmega16
F_CPU=16500000UL   # running at 8 Mhz

CXX=avr-g++
CPPFLAGS=-std=c++17 -Wall -Wextra -Os -DF_CPU=${F_CPU} -mmcu=${MCU}

${PROJECT}.hex: ${PROJECT}.elf
	avr-objcopy -j .text -j .data -O ihex $< $@
	arm-linux-gnueabihf-size ${PROJECT}.hex

${PROJECT}.elf: ${OBJECTS}
	$(CXX) -mmcu=${MCU} -o $@ $^

test-connection:
	sudo avrdude ${AVRDUDE_FLAGS}

clean:
	rm -f *.elf *.o *.hex

upload: ${PROJECT}.hex
	sudo avrdude ${AVRDUDE_FLAGS} -U flash:w:$<:i

fuse:
	sudo avrdude ${AVRDUDE_FLAGS} ${FUSES}

# vim: set ts=8 sts=8 sw=8 noexpandtab:
