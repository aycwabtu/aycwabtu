
.PHONY: all clean

CC          = gcc
LD          = gcc

OBJDIR := obj

CFLAGS      = -O2

obj/%.o : %.c | $(OBJDIR)
	$(CC) -c $(CFLAGS) -o obj/$*.o $<

src = \
	dvbcsa_algo.c	  \
	dvbcsa_block.c    \
	dvbcsa_key.c      \
	dvbcsa_stream.c
	

obj = $(src:%.c=$(OBJDIR)/%.o)

all: dvbcsa/libdvbcsa.a
   
dvbcsa/libdvbcsa.a: $(obj)
	#$(LD) -static -o $@ $(obj)
	ar rcs dvbcsa/libdvbcsa.a $(obj)
	@echo $@ created

-include $(obj:.o=.d)

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	@rm -rf obj

