PROJECT=z80_controller
OBJECTS=main.o serial.o lowlevel.o bus.o z80.o wait.o memory.o repl.o tests.o debugger.o disassembler.o programatic.o run.o
AVRDUDE_FLAGS=-p atmega16 -C ./avrdude_gpio.conf -c pi_1 -B 0.5 -V
#FUSES=-U lfuse:w:0xe4:m -U hfuse:w:0xd9:m   # internal 8 Mhz
FUSES=-U lfuse:w:0xff:m -U hfuse:w:0xd9:m    # external crystal
MCU=atmega16
F_CPU=16500000UL   # running at 16.5 Mhz

CC=avr-gcc
CPPFLAGS=-std=c11 -Wall -Wextra -Os -DF_CPU=${F_CPU} -mmcu=${MCU} -ffunction-sections -fdata-sections -DADD_TESTS=1 -DADD_DEBUGGER=1

${PROJECT}.hex: ${PROJECT}.elf
	avr-objcopy -j .text -j .data -O ihex $< $@
	arm-linux-gnueabihf-size ${PROJECT}.hex

${PROJECT}.elf: ${OBJECTS}
	$(CC) -mmcu=${MCU} -o $@ $^ -Wl,--gc-sections
	avr-size -C --mcu=${MCU} ${PROJECT}.elf	

test-disasm: test_disasm.c disassembler.c
	gcc -ggdb3 -O0 -Wall -Wextra -std=c11 -o $@ $^ -DTEST=1

test-simulator: programatic.c test_simulator.c
	gcc -ggdb3 -O0 -Wall -Wextra -std=c11 -o $@ $^ -DSIMULATOR=1 -lutil

test: test-disasm
	./$^

test-connection:
	sudo avrdude ${AVRDUDE_FLAGS}

clean:
	rm -f *.elf *.o *.hex test-disasm test-simulator

upload: ${PROJECT}.hex
	sudo avrdude ${AVRDUDE_FLAGS} -U flash:w:$<:i

fuse:
	sudo avrdude ${AVRDUDE_FLAGS} ${FUSES}

# vim: set ts=8 sts=8 sw=8 noexpandtab:
