# get the type of OS currently running
OS=$(shell uname)
PWD=$(shell pwd)

CC    = gcc
CXX   = g++
F90   = gfortran
INC   = -I./src
LIBS  =
CLIBS = -lc++
DEFS  =

CXXFLAGS = -O2 -funroll-loops -fPIC

SRCS       = $(shell echo src/*.cc) $(shell echo src/fmt/*.cc)
OBJS       = $(SRCS:.cc=.o)
SRCS_TESTS = $(shell echo src_tests/*.cc)
OBJS_TESTS = $(SRCS_TESTS:.cc=.o)

#src/AlglinConfig.hh
DEPS = $(shell echo src/*.h*)

# check if the OS string contains 'Linux'
ifneq (,$(findstring Linux, $(OS)))
include Makefile_linux.mk
endif

# check if the OS string contains 'MINGW'
ifneq (,$(findstring MINGW, $(OS)))
include Makefile_mingw.mk
endif

# check if the OS string contains 'Darwin'
ifneq (,$(findstring Darwin, $(OS)))
include Makefile_osx.mk
endif

MKDIR = mkdir -p

.SUFFIXES:           # Delete the default suffixes
.SUFFIXES: .c .cc .o # Define our suffix list

# prefix for installation, use make PREFIX=/new/prefix install
# to override
PREFIX = /usr/local

tests: all_libs $(OBJS_TESTS)
	mkdir -p bin
	$(CXX) $(INC) $(DEFS) $(CXXFLAGS) -o bin/test1-small-factorization src_tests/test1-small-factorization.o  $(ALL_LIBS) $(LIBSGCC)
	$(CXX) $(INC) $(DEFS) $(CXXFLAGS) -o bin/test2-Timing              src_tests/test2-Timing.o               $(ALL_LIBS) $(LIBSGCC)
	$(CXX) $(INC) $(DEFS) $(CXXFLAGS) -o bin/test3-BandedMatrix        src_tests/test3-BandedMatrix.o         $(ALL_LIBS) $(LIBSGCC)
	$(CXX) $(INC) $(DEFS) $(CXXFLAGS) -o bin/test4-BFGS                src_tests/test4-BFGS.o                 $(ALL_LIBS) $(LIBSGCC)
	$(CXX) $(INC) $(DEFS) $(CXXFLAGS) -o bin/test5-BLOCKTRID           src_tests/test5-BLOCKTRID.o            $(ALL_LIBS) $(LIBSGCC)
	$(CXX) $(INC) $(DEFS) $(CXXFLAGS) -o bin/test6-EIGS                src_tests/test6-EIGS.o                 $(ALL_LIBS) $(LIBSGCC)

.cc.o:
	$(CXX) $(INC) $(CXXFLAGS) $(DEFS) -c $< -o $@

.c.o:
	$(CC) $(INC) $(CFLAGS) $(DEFS) -c $< -o $@

install_local: all_libs
	$(MKDIR) ./lib
	$(MKDIR) ./lib/include
	$(MKDIR) ./lib/include/fmt
	$(MKDIR) ./lib/include/zstream
	cp -f -P src/*.h*         ./lib/include
	cp -f -P src/fmt/*.h*     ./lib/include/fmt
	cp -f -P src/zstream/*.h* ./lib/include/zstream

install: all_libs
	$(MKDIR) $(PREFIX)/include
	cp -rf lib/include/* $(PREFIX)/include
	cp -f -P lib/lib/*   $(PREFIX)/lib

run: tests
	./bin/test1-small-factorization
	./bin/test2-Timing
	./bin/test3-BandedMatrix
	./bin/test4-BFGS
	./bin/test5-BLOCKTRID
	./bin/test6-EIGS

doc:
	doxygen

clean:
	rm -rf lib/* src/*.o src/*/*.o src/*/*/*.o src_tests/*.o src/*.obj src/*/*.obj src/*/*/*.obj src_tests/*.obj
	rm -rf bin

depend:
	makedepend -- $(INC) $(CXXFLAGS) $(DEFS) -- $(SRCS)
# DO NOT DELETE

src/Console.o: src/Utils.hh src/rang.hpp src/fmt/printf.h src/fmt/ostream.h
src/Console.o: src/fmt/format.h src/fmt/core.h src/Trace.hxx src/Console.hxx
src/Console.o: src/Malloc.hxx src/Numbers.hxx src/TicToc.hxx
src/Console.o: src/ThreadPool.hxx
src/Malloc.o: src/Utils.hh src/rang.hpp src/fmt/printf.h src/fmt/ostream.h
src/Malloc.o: src/fmt/format.h src/fmt/core.h src/Trace.hxx src/Console.hxx
src/Malloc.o: src/Malloc.hxx src/Numbers.hxx src/TicToc.hxx
src/Malloc.o: src/ThreadPool.hxx
src/Numbers.o: src/Utils.hh src/rang.hpp src/fmt/printf.h src/fmt/ostream.h
src/Numbers.o: src/fmt/format.h src/fmt/core.h src/Trace.hxx src/Console.hxx
src/Numbers.o: src/Malloc.hxx src/Numbers.hxx src/TicToc.hxx
src/Numbers.o: src/ThreadPool.hxx
src/Trace.o: src/Utils.hh src/rang.hpp src/fmt/printf.h src/fmt/ostream.h
src/Trace.o: src/fmt/format.h src/fmt/core.h src/Trace.hxx src/Console.hxx
src/Trace.o: src/Malloc.hxx src/Numbers.hxx src/TicToc.hxx src/ThreadPool.hxx
src/Utils.o: src/Utils.hh src/rang.hpp src/fmt/printf.h src/fmt/ostream.h
src/Utils.o: src/fmt/format.h src/fmt/core.h src/Trace.hxx src/Console.hxx
src/Utils.o: src/Malloc.hxx src/Numbers.hxx src/TicToc.hxx src/ThreadPool.hxx
src/fmt/format.o: ./src/fmt/format-inl.h src/fmt/format.h src/fmt/core.h
src/fmt/os.o: ./src/fmt/os.h src/fmt/format.h src/fmt/core.h
