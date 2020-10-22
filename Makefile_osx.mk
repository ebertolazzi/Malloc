WARN     = -Wall
CC       = clang
CXX      = clang++
VERSION  = $(shell $(CC) --version 2>&1 | grep -o "Apple LLVM version [0-9]\.[0-9]\.[0-9]" | grep -o " [0-9]\.")
#---------
CXX     += -std=c++11 -stdlib=libc++ -g

#---------
CC     += $(WARN)
CXX    += $(WARN)
AR      = libtool -static -o
LIBSGCC = -lstdc++ -lm

ALL_LIBS = $(LIBS) -Llib/lib -Llib/dll

all_libs: lib/dll/libUtils_osx.dylib lib/lib/libUtils_osx_static.a

lib/dll/libUtils_osx.dylib: $(OBJS)
	@$(MKDIR) lib
	@$(MKDIR) lib/lib
	@$(MKDIR) lib/dll
	@$(MKDIR) lib/bin
	$(CXX) -dynamiclib -o lib/dll/libUtils_osx.dylib $(OBJS) $(LIBS)

lib/lib/libUtils_osx_static.a: $(OBJS)
	@$(MKDIR) lib
	@$(MKDIR) lib/lib
	@$(MKDIR) lib/dll
	@$(MKDIR) lib/bin
	libtool -static $(OBJS) -o lib/lib/libUtils_osx_static.a
