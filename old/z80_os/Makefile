BIN=z80aw_os.bin

AS=vasmz80_oldstyle

ASFLAGS=-chklabels -L listing.txt -Fbin -autoexp

.PHONY: $(BIN)
$(BIN): mmap.z80
	$(AS) $(ASFLAGS) -o $@ $^
	./bioscalls.sh > common/bios.z80
	wc -c $@

upload: $(BIN)
	hexdump -C $^
	-screen -S z80 -X quit
	../reset
	../upload < $^

.PHONY: clean
clean:
	rm -f $(BIN)

# vim: set ts=8 sts=8 sw=8 noexpandtab:
