CFLAGS = -Wall BF_64.a -no-pie

ODIR = ./obj

_DEPS = hash_table sec_hash_table error_messages record_struct
DEPS := $(foreach D,$(_DEPS),./$D/$D.h) ./BF.h
INCLUDE_HEADERS = $(patsubst %,-I ./%,$(_DEPS)) -I ./

_HT_OBJS = main_SHT sec_hash_table hash_table
HT_OBJS = $(patsubst %,$(ODIR)/%.o,$(_HT_OBJS))

EXEC = main_sht

# Main executables
all: $(EXEC)

main_sht: $(HT_OBJS)
	gcc -o $@ $^ $(CFLAGS)
	@echo "Executable file \"$@\" created";

# Main programs
$(ODIR)/main_SHT.o: ./main_sht_/main_SHT.c $(DEPS) | $(ODIR)
	gcc -c -o $@ $< $(INCLUDE_HEADERS) $(CFLAGS)

# Utilities & data structures
$(ODIR)/sec_hash_table.o: ./sec_hash_table/sec_hash_table.c $(DEPS) | $(ODIR)
	gcc -c -o $@ $< $(INCLUDE_HEADERS) $(CFLAGS)

$(ODIR)/hash_table.o: ./hash_table/hash_table.c $(DEPS) | $(ODIR)
	gcc -c -o $@ $< $(INCLUDE_HEADERS) $(CFLAGS)
# Objects directory
$(ODIR):
	mkdir -p $@

# Make clean
.PHONY: clean

clean:
	rm -rf $(ODIR)/
	rm -f $(EXEC)
