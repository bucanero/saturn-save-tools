# Checksum Fixer
# (c)2020 Damian Parrino

all: unpacker converter

unpacker:
	gcc -D _GNU_SOURCE -I./include -o sgm-unpacker src/sgm-unpacker.c src/bzlib.c src/blocksort.c src/crctable.c src/decompress.c src/compress.c src/huffman.c src/randtable.c

converter:
	gcc -D _GNU_SOURCE -I./include -o ss-converter src/ss-converter.c src/xml.c

clean:
	-rm -f sgm-unpacker save-converter
