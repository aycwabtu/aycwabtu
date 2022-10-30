.PHONY: all clean

CC          = gcc
LD          = gcc

SHELL=bash

GITHASH := $(shell git rev-parse --short HEAD)

CFLAGS      = \
    -w                                  \
    -I src/libdvbcsa/dvbcsa             \
    -msse2                              \
    -O2                                 \
    -DGITHASH=\"$(GITHASH)\" 

obj/%.o : src/%.c
	@mkdir -p $(@D)
	$(CC) -c -MD $(CFLAGS)-o obj/$*.o $<

ayc_src = \
	aycwabtu_main.c             \
	aycwabtu_bs_algo.c          \
	aycwabtu_bs_block.c         \
	aycwabtu_bs_block_ab.c      \
	aycwabtu_bs_sse2.c     		\
	aycwabtu_bs_stream.c        \
	aycwabtu_bs_uint32.c	    \
	aycwabtu_ts.c

tsgen_src = tsgen.c

libdvbcsa_src = \
	libdvbcsa/dvbcsa_algo.c     \
	libdvbcsa/dvbcsa_block.c    \
	libdvbcsa/dvbcsa_key.c      \
	libdvbcsa/dvbcsa_stream.c

ayc_obj         = $(ayc_src:%.c=obj/%.o)
tsgen_obj       = $(tsgen_src:%.c=obj/%.o)
libdvbcsa_obj   = $(libdvbcsa_src:%.c=obj/%.o)

all: aycwabtu
   

aycwabtu: $(ayc_obj) $(libdvbcsa_obj)
	$(LD) -static -s -o $@ $(ayc_obj) $(libdvbcsa_obj)
	@echo $@ created

tsgen: $(tsgen_obj) $(libdvbcsa_obj)
	$(LD) -o $@ $(tsgen_obj) $(libdvbcsa_obj)
	@echo $@ created


test: aycwabtu tsgen always
	cd test && ./testframe.sh | tee testframe.log

always:


aycwabtu tsgen : makefile

include $(wildcard obj/*.d) $(wildcard obj/libdvbcsa/*.d)

clean:
	@rm -rf aycwabtu tsgen aycwabtu.exe tsgen.exe obj

