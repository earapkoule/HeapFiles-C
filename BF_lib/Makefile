CFLAGS = -Wall -pthread

ODIR = ./obj

_DEPS = hash_table heap_file error_messages record_struct
DEPS := $(foreach D,$(_DEPS),./$D/$D.h)
INCLUDE_HEADERS = $(patsubst %,-I ./%,$(_DEPS))

_HP_OBJS = main_hp heap_file error_messages record_struct
HP_OBJS = $(patsubst %,$(ODIR)/%.o,$(_HP_OBJS))

_HT_OBJS = main_ht hash_table error_messages record_struct
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
$(ODIR)/main_hp.o: ./main_hp_/main_hp.c $(DEPS) | $(ODIR)
	gcc -c -o $@ $< $(INCLUDE_HEADERS) $(CFLAGS)

$(ODIR)/main_ht.o: ./main_ht_/main_ht.c $(DEPS) | $(ODIR)
	gcc -c -o $@ $< $(INCLUDE_HEADERS) $(CFLAGS)

# Utilities & data structures
$(ODIR)/error_messages.o: ./error_messages/error_messages.c $(DEPS) | $(ODIR)
	gcc -c -o $@ $< $(INCLUDE_HEADERS) $(CFLAGS)
$(ODIR)/record_struct.o: ./record_struct/record_struct.c $(DEPS) | $(ODIR)
	gcc -c -o $@ $< $(INCLUDE_HEADERS) $(CFLAGS)

# Objects directory
$(ODIR):
	mkdir -p $@

# Make clean
.PHONY: clean

clean:
	rm -rf $(ODIR)/
	rm -f $(EXEC)
