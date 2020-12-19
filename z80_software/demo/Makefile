AS=vasmz80_std
LD=vlink

ASFLAGS=-chklabels -nocase -Llisting.txt -Fbin

demo.bin: demo.z80
	$(AS) $(ASFLAGS) -o $@ $^

upload-demo: demo.bin
	hexdump -C $^
	-screen -S z80 -X quit
	../../reset
	../../upload < $^

.PHONY: clean
clean:
	rm -f demo.bin

# vim: set ts=8 sts=8 sw=8 noexpandtab:
