#!/bin/sh

grep '^[A-Z][A-Z_]\+ EXPR' listing.txt | sed 's/^\([A-Z0-9_]\+\) EXPR([0-9]\+\(=0x[0-9a-f]\+\).*/\1\2/g'
