PROJECT=firmware-controller
OBJECTS=main.o serial.o repl.o
AVRDUDE_FLAGS=-p atmega16 -C ./avrdude_gpio.conf -c pi_1 -B 0.5 -V
FUSES=-U lfuse:w:0xff:m -U hfuse:w:0xd9:m    # external crystal
MCU=atmega16
F_CPU=16500000UL   # running at 16.5 Mhz

CC=avr-gcc
CXX=avr-g++
CXXFLAGS=-std=c++17
CPPFLAGS=-Wall -Wextra -Os -DF_CPU=${F_CPU} -mmcu=${MCU} -ffunction-sections -fdata-sections

all: ${PROJECT}.hex

${PROJECT}.hex: ${PROJECT}.elf
	avr-objcopy -j .text -j .data -O ihex $< $@
	arm-linux-gnueabihf-size ${PROJECT}.hex

${PROJECT}.elf: ${OBJECTS}
	$(CC) -mmcu=${MCU} -o $@ $^ -Wl,--gc-sections
	avr-size -C --mcu=${MCU} ${PROJECT}.elf	

test-connection:
	sudo avrdude ${AVRDUDE_FLAGS}

clean:
	rm -f *.elf *.o *.hex test-disasm test-simulator

upload: ${PROJECT}.hex
	sudo avrdude ${AVRDUDE_FLAGS} -U flash:w:$<:i

fuse:
	sudo avrdude ${AVRDUDE_FLAGS} ${FUSES}

# vim: set ts=8 sts=8 sw=8 noexpandtab:
