demo.bin: demo.z80
	z80asm -o $@ $^

upload-demo: demo.bin
	hexdump -C $^
	-screen -d
	make -C ../firmware test-connection  # reset AVR
	../reset
	../upload < $^

# vim: set ts=8 sts=8 sw=8 noexpandtab:
