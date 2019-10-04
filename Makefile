
#
# Makefile - membench
#
# author: srikanth yalavarthi
# last modified: 2019-10-02
# url: https://github.com/srikanthy/membench.git
#

TOOLCHAIN		= gcc

ifeq ($(TOOLCHAIN), gcc)
CPP         = /lib/cpp -P
CC          = gcc
ARCH_FLAGS  =
OMP_FLAGS   = -fopenmp
DEBUG_FLAGS = -std=c11 -pedantic -g -Wall -Wextra --save-temps
OPT_FLAGS   = $(OMP_FLAGS) -O0 -fno-tree-vectorize
endif

ifeq ($(TOOLCHAIN), llvm)
CPP         = clang -E
CC          = clang
ARCH_FLAGS  =
OMP_FLAGS   = -fopenmp
DEBUG_FLAGS = -std=c11 -pedantic -g -Wall -Wextra --save-temps
OPT_FLAGS   = $(OMP_FLAGS) -O0 -fno-vectorize
endif

INCDIR			=
LIBDIR			=
LIBS        =
CPPFLAGS    = $(ARCH_FLAGS) $(INCDIR)
CFLAGS      = $(DEBUG_FLAGS) $(OPT_FLAGS)
LDFLAGS     = $(LIBDIR) $(LIBS) $(OMP_FLAGS)
RM          = rm -rf
LN          = ln -sf

# source files
LIB_HDR =
LIB_SRC =
EXE_SRC = membench.c perf_events.c perf_events_multi.c perf_events_loop.c

# derived files
EXE = $(EXE_SRC:.c=.x)
PPS = $(EXE_SRC:.c=.i) $(LIB_SRC:.c=.i)
ASM = $(EXE_SRC:.c=.s) $(LIB_SRC:.c=.s)
OBJ = $(EXE_SRC:.c=.o) $(LIB_SRC:.c=.o)

# build targets
all: $(EXE)

membench.x: membench.o $(LIB_SRC:.c=.o)
	$(CC) $(LDFLAGS) -o $@ $^

perf_events.o: CFLAGS += -Wno-pedantic
perf_events.x: perf_events.o $(LIB_SRC:.c=.o)
	$(CC) $(LDFLAGS) -o $@ $^

perf_events_multi.o: CFLAGS += -Wno-pedantic
perf_events_multi.x: perf_events_multi.o $(LIB_SRC:.c=.o)
	$(CC) $(LDFLAGS) -o $@ $^

perf_events_loop.o: CFLAGS += -Wno-pedantic
perf_events_loop.x: perf_events_loop.o $(LIB_SRC:.c=.o)
	$(CC) $(LDFLAGS) -o $@ $^

# build rules
%.o : %.c
	$(CPP) $(CPPFLAGS) $< > $*.i
	$(CC) $(CFLAGS) -c $*.i -o $*.o

# additional rules
clean:
	$(RM) $(OBJ)
	$(RM) $(ASM)
	$(RM) $(PPS)

distclean: clean
	$(RM) $(EXE)
	$(RM) membench.csv
	$(RM) membench.pdf
