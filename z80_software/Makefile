demo.bin: demo.z80
	z80asm -o $@ $^

upload-demo: demo.bin
	hexdump -C $^
	-killall -9 screen
	make -C ../firmware test-connection  # reset AVR
	../upload < $^

# vim: set ts=8 sts=8 sw=8 noexpandtab:
