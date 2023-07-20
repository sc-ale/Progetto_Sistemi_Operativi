# Cross toolchain variables
# If these are not in your path, you can make them absolute.
XT_PRG_PREFIX = mipsel-linux-gnu-
CC = $(XT_PRG_PREFIX)gcc
LD = $(XT_PRG_PREFIX)ld

# uMPS3-related paths

# Simplistic search for the umps3 installation prefix.
# If you have umps3 installed on some weird location, set UMPS3_DIR_PREFIX by hand.
ifneq ($(wildcard /usr/bin/umps3),)
	UMPS3_DIR_PREFIX = /usr
else
	UMPS3_DIR_PREFIX = /usr/local
endif

UMPS3_DATA_DIR = $(UMPS3_DIR_PREFIX)/share/umps3
UMPS3_INCLUDE_DIR = $(UMPS3_DIR_PREFIX)/include/umps3
UMPS3_UTILITY_DIR = ./utility/
UMPS3_PHASE1_DIR = ./phase1/
UMPS3_PANDOS_DIR = ./

# Compiler options
CFLAGS_LANG = -ffreestanding -ansi
CFLAGS_MIPS = -mips1 -mabi=32 -mno-gpopt -EL -G 0 -mno-abicalls -fno-pic -mfp32 -Wall
CFLAGS = $(CFLAGS_LANG) $(CFLAGS_MIPS) -I$(UMPS3_INCLUDE_DIR) -I$(UMPS3_PANDOS_DIR) -I$(UMPS3_UTILITY_DIR) -I$(UMPS3_PHASE1_DIR) -O0 -std=gnu99

# Linker options
LDFLAGS = -G 0 -nostdlib -T $(UMPS3_DATA_DIR)/umpscore.ldscript -m elf32ltsmip

# Add the location of crt*.S to the search path
VPATH = $(UMPS3_DATA_DIR)

.PHONY : all clean

all : ./machine/kernel.core.umps

./machine/kernel.core.umps : ./machine/kernel
	umps3-elf2umps -k $<

./machine/kernel : p2test.04.o crtso.o libumps.o initial.o exception.o scheduler.o interrupt.o ./phase1/pcb.o ./phase1/ash.o ./phase1/ns.o ./utility/memcpy.o
	$(LD) -o $@ $^ $(LDFLAGS)

clean :
	-rm -f *.o ./machine/kernel ./machine/kernel.*.umps ./phase1/*.o ./utility/*.o

# Pattern rule for assembly modules
%.o : %.S
	$(CC) $(CFLAGS) -c -o $@ $<

