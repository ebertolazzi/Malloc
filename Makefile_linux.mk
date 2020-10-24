WARN   = -Wall
CC     = gcc -fPIC
CXX    = g++ -fPIC -std=c++11 -pthread -g
#
# activate C++11 for g++ >= 4.9
#
VERSION  = $(shell $(CC) -dumpversion)
CC     += $(WARN)
CXX    += $(WARN)
AR      = ar rcs
LIBSGCC = -static-libgcc -static-libstdc++ -lm -ldl

override CXXFLAGS += -floop-interchange -floop-block

ALL_LIBS = $(LIBS) -Llib/lib -Llib/dll -lUtils_linux_static

all_libs: lib/dll/libUtils_linux.so lib/lib/libUtils_linux_static.a

lib/dll/libUtils_linux.so: $(OBJS)
	@$(MKDIR) lib
	@$(MKDIR) lib/dll
	$(CXX) -shared -o lib/dll/libUtils_linux.so $(OBJS) $(LIBS)

lib/lib/libUtils_linux_static.a: $(OBJS)
	@$(MKDIR) lib
	@$(MKDIR) lib/lib
	$(AR) lib/lib/libUtils_linux_static.a $(OBJS)
