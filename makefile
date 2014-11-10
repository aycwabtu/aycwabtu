.PHONY: all clean

CC          = gcc
LD          = gcc

SHELL=bash

OBJDIR := obj

DVBCSAINC 	:= libdvbcsa/dvbcsa
DVBCSALIB 	:= $(DVBCSAINC)/libdvbcsa.a

CFLAGS      =  -w -I $(DVBCSAINC) -msse2 -O2
#CFLAGS      =  -w -I $(DVBCSAINC) -msse2 -D_DEBUG

obj/%.o : %.c | $(OBJDIR)
	@if [ "$<" == "aycwabtu_main.c" ] ; then (echo -n "#define GITSHA1 \"`git rev-parse --short=16 HEAD`\"") >aycwabtu_version.h; echo "aycwabtu_version.h written"; fi;
	$(CC) -c -MD $(CFLAGS) -o obj/$*.o $<

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

ayc_obj = $(ayc_src:%.c=obj/%.o)
tsgen_obj = $(tsgen_src:%.c=obj/%.o)

all: aycwabtu
   
$(DVBCSALIB):
	@echo making target 'all' in $(DVBCSAMAKE)
	make --directory=libdvbcsa all

aycwabtu: $(ayc_obj) $(DVBCSALIB)
	$(LD) -static -s -o $@ $(ayc_obj) -static -L. -ldvbcsa/dvbcsa/libdvbcsa
	@echo $@ created

tsgen: $(tsgen_obj) $(DVBCSALIB)
	$(LD) -o $@ $(tsgen_obj) -static -L. -ldvbcsa/dvbcsa/libdvbcsa
	@echo $@ created


test: aycwabtu tsgen always
	cd test && ./testframe.sh | tee testframe.log

always:

# pull in dependency info for *existing* .o files
-include $(ayc_obj:.o=.d)
-include $(tsgen_obj:.o=.d)

$(ayc_obj) $(tsgen_obj) : makefile

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	@rm -rf aycwabtu tsgen aycwabtu.exe tsgen.exe obj
	@make -s --directory=libdvbcsa clean

