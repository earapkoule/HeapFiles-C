CFLAGS = BF_64.a -no-pie

ODIR = ./obj

_DEPS = hash_table heap_file error_messages record_struct
DEPS := $(foreach D,$(_DEPS),./$D/$D.h) ./BF.h
INCLUDE_HEADERS = $(patsubst %,-I ./%,$(_DEPS)) -I ./

_HP_OBJS = main_HP heap_file
HP_OBJS = $(patsubst %,$(ODIR)/%.o,$(_HP_OBJS))

_HT_OBJS = main_HT hash_table
HT_OBJS = $(patsubst %,$(ODIR)/%.o,$(_HT_OBJS))

EXEC = main_hp main_ht

# Main executables
all: $(EXEC)

main_hp: $(HP_OBJS)
	gcc -o $@ $^ $(CFLAGS)
	@echo "Executable file \"$@\" created";

main_ht: $(HT_OBJS)
	gcc -o $@ $^ $(CFLAGS)
	@echo "Executable file \"$@\" created";

# Main programs
$(ODIR)/main_HP.o: ./main_hp_/main_HP.c $(DEPS) | $(ODIR)
	gcc -c -o $@ $< $(INCLUDE_HEADERS) $(CFLAGS)

$(ODIR)/main_HT.o: ./main_ht_/main_HT.c $(DEPS) | $(ODIR)
	gcc -c -o $@ $< $(INCLUDE_HEADERS) $(CFLAGS)

# Utilities & data structures
$(ODIR)/heap_file.o: ./heap_file/heap_file.c $(DEPS) | $(ODIR)
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
