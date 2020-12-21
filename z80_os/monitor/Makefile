BIN=monitor.bin

AS=vasmz80_oldstyle

ASFLAGS=-chklabels -L listing.txt -Fbin
ADDRESS=0x7000

.PHONY: $(BIN)
$(BIN): monitor.z80
	$(AS) $(ASFLAGS) -o $@ $^
	wc -c $@

upload: $(BIN)
	hexdump -C $^
	-screen -S z80 -X quit
	../../reset
	../../upload --dest $(ADDRESS) < $^

.PHONY: clean
clean:
	rm -f $(BIN)

# vim: set ts=8 sts=8 sw=8 noexpandtab:
