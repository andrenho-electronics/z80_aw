BIN=z80aw_os.bin

AS=vasmz80_std
LD=vlink

ASFLAGS=-chklabels -nocase -Llisting.txt -Fbin

.PHONY: $(BIN)
$(BIN): z80aw_os.z80
	$(AS) $(ASFLAGS) -o $@ $^
	wc -c $@

upload: $(BIN)
	hexdump -C $^
	-screen -S z80 -X quit
	../../reset
	../../upload < $^

.PHONY: clean
clean:
	rm -f $(BIN)

# vim: set ts=8 sts=8 sw=8 noexpandtab:
