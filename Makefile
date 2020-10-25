# get the type of OS currently running
OS=$(shell uname)
PWD=$(shell pwd)

CC    = gcc
CXX   = g++
F90   = gfortran
INC   = -Isrc -Isrc/Utils
LIBS  =
CLIBS = -lc++
DEFS  =

CXXFLAGS = -O2 -funroll-loops -fPIC
CFLAGS   = -O2 -funroll-loops -fPIC

SRCS       = $(shell echo src/*.cc) $(shell echo src/Utils/fmt/*.cc)
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
	$(CXX) $(INC) $(DEFS) $(CXXFLAGS) -o bin/test_trace src_tests/test_trace.cc $(ALL_LIBS) $(LIBSGCC)
	$(CXX) $(INC) $(DEFS) $(CXXFLAGS) -o bin/test_Malloc src_tests/test_Malloc.cc $(ALL_LIBS) $(LIBSGCC)

.cc.o:
	$(CXX) $(INC) $(CXXFLAGS) $(DEFS) -c $< -o $@

.c.o:
	$(CC) $(INC) $(CFLAGS) $(DEFS) -c $< -o $@

install_local: all_libs
	$(MKDIR) ./lib
	$(MKDIR) ./lib/include
	$(MKDIR) ./lib/include/fmt
	$(MKDIR) ./lib/include/zstream
	cp -f -P src/*.h*               ./lib/include
	cp -f -P src/Utils/*.h*         ./lib/include/Utils
	cp -f -P src/Utils/fmt/*.h*     ./lib/include/Utils/fmt
	cp -f -P src/Utils/zstream/*.h* ./lib/include/Utils/zstream

install: all_libs
	$(MKDIR) $(PREFIX)/include
	cp -rf lib/include/* $(PREFIX)/include
	cp -f -P lib/lib/*   $(PREFIX)/lib

run: tests
	./bin/test_trace
	./bin/test_Malloc

doc:
	doxygen

clean:
	rm -rf lib/* src/*.o src/*/*.o src/*/*/*.o src_tests/*.o src/*.obj src/*/*.obj src/*/*/*.obj src_tests/*.obj
	rm -rf bin

travis: all_libs tests
	make run

depend:
	makedepend -- $(INC) $(CXXFLAGS) $(DEFS) -- $(SRCS)
# DO NOT DELETE

src/Console.o: src/Utils.hh src/Utils/Utils.hxx src/Utils/rang.hpp
src/Console.o: src/Utils/fmt/printf.h src/Utils/fmt/ostream.h
src/Console.o: src/Utils/fmt/format.h src/Utils/fmt/core.h
src/Console.o: src/Utils/fmt/chrono.h src/Utils/fmt/locale.h
src/Console.o: src/Utils/fmt/ostream.h src/Utils/zstream/izstream.hpp
src/Console.o: src/Utils/zstream/zstream_common.hpp
src/Console.o: src/Utils/zstream/izstream_impl.hpp
src/Console.o: src/Utils/zstream/izstream.hpp src/Utils/zstream/ozstream.hpp
src/Console.o: src/Utils/zstream/ozstream_impl.hpp
src/Console.o: src/Utils/zstream/ozstream.hpp src/Utils/Trace.hxx
src/Console.o: src/Utils/Console.hxx src/Utils/Malloc.hxx
src/Console.o: src/Utils/Numbers.hxx src/Utils/TicToc.hxx
src/Console.o: src/Utils/ThreadPool.hxx
src/Malloc.o: src/Utils.hh src/Utils/Utils.hxx src/Utils/rang.hpp
src/Malloc.o: src/Utils/fmt/printf.h src/Utils/fmt/ostream.h
src/Malloc.o: src/Utils/fmt/format.h src/Utils/fmt/core.h
src/Malloc.o: src/Utils/fmt/chrono.h src/Utils/fmt/locale.h
src/Malloc.o: src/Utils/fmt/ostream.h src/Utils/zstream/izstream.hpp
src/Malloc.o: src/Utils/zstream/zstream_common.hpp
src/Malloc.o: src/Utils/zstream/izstream_impl.hpp
src/Malloc.o: src/Utils/zstream/izstream.hpp src/Utils/zstream/ozstream.hpp
src/Malloc.o: src/Utils/zstream/ozstream_impl.hpp
src/Malloc.o: src/Utils/zstream/ozstream.hpp src/Utils/Trace.hxx
src/Malloc.o: src/Utils/Console.hxx src/Utils/Malloc.hxx
src/Malloc.o: src/Utils/Numbers.hxx src/Utils/TicToc.hxx
src/Malloc.o: src/Utils/ThreadPool.hxx
src/Numbers.o: src/Utils.hh src/Utils/Utils.hxx src/Utils/rang.hpp
src/Numbers.o: src/Utils/fmt/printf.h src/Utils/fmt/ostream.h
src/Numbers.o: src/Utils/fmt/format.h src/Utils/fmt/core.h
src/Numbers.o: src/Utils/fmt/chrono.h src/Utils/fmt/locale.h
src/Numbers.o: src/Utils/fmt/ostream.h src/Utils/zstream/izstream.hpp
src/Numbers.o: src/Utils/zstream/zstream_common.hpp
src/Numbers.o: src/Utils/zstream/izstream_impl.hpp
src/Numbers.o: src/Utils/zstream/izstream.hpp src/Utils/zstream/ozstream.hpp
src/Numbers.o: src/Utils/zstream/ozstream_impl.hpp
src/Numbers.o: src/Utils/zstream/ozstream.hpp src/Utils/Trace.hxx
src/Numbers.o: src/Utils/Console.hxx src/Utils/Malloc.hxx
src/Numbers.o: src/Utils/Numbers.hxx src/Utils/TicToc.hxx
src/Numbers.o: src/Utils/ThreadPool.hxx
src/Trace.o: src/Utils.hh src/Utils/Utils.hxx src/Utils/rang.hpp
src/Trace.o: src/Utils/fmt/printf.h src/Utils/fmt/ostream.h
src/Trace.o: src/Utils/fmt/format.h src/Utils/fmt/core.h
src/Trace.o: src/Utils/fmt/chrono.h src/Utils/fmt/locale.h
src/Trace.o: src/Utils/fmt/ostream.h src/Utils/zstream/izstream.hpp
src/Trace.o: src/Utils/zstream/zstream_common.hpp
src/Trace.o: src/Utils/zstream/izstream_impl.hpp
src/Trace.o: src/Utils/zstream/izstream.hpp src/Utils/zstream/ozstream.hpp
src/Trace.o: src/Utils/zstream/ozstream_impl.hpp
src/Trace.o: src/Utils/zstream/ozstream.hpp src/Utils/Trace.hxx
src/Trace.o: src/Utils/Console.hxx src/Utils/Malloc.hxx src/Utils/Numbers.hxx
src/Trace.o: src/Utils/TicToc.hxx src/Utils/ThreadPool.hxx
src/Utils.o: src/Utils.hh src/Utils/Utils.hxx src/Utils/rang.hpp
src/Utils.o: src/Utils/fmt/printf.h src/Utils/fmt/ostream.h
src/Utils.o: src/Utils/fmt/format.h src/Utils/fmt/core.h
src/Utils.o: src/Utils/fmt/chrono.h src/Utils/fmt/locale.h
src/Utils.o: src/Utils/fmt/ostream.h src/Utils/zstream/izstream.hpp
src/Utils.o: src/Utils/zstream/zstream_common.hpp
src/Utils.o: src/Utils/zstream/izstream_impl.hpp
src/Utils.o: src/Utils/zstream/izstream.hpp src/Utils/zstream/ozstream.hpp
src/Utils.o: src/Utils/zstream/ozstream_impl.hpp
src/Utils.o: src/Utils/zstream/ozstream.hpp src/Utils/Trace.hxx
src/Utils.o: src/Utils/Console.hxx src/Utils/Malloc.hxx src/Utils/Numbers.hxx
src/Utils.o: src/Utils/TicToc.hxx src/Utils/ThreadPool.hxx
src/Utils/fmt/format.o: src/Utils/fmt/format-inl.h src/Utils/fmt/format.h
src/Utils/fmt/format.o: src/Utils/fmt/core.h
src/Utils/fmt/os.o: src/Utils/fmt/os.h src/Utils/fmt/format.h
src/Utils/fmt/os.o: src/Utils/fmt/core.h
