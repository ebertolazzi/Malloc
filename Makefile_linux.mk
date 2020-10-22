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

ALL_LIBS = $(LIBS) -Llib/lib -Llib/dll -llapack_wrapper_linux -lHSL_linux

all_libs: lib/dll/libUtils.so lib/lib/libUtils.a

lib/dll/libUtils.so: $(OBJS)
	@$(MKDIR) lib
	@$(MKDIR) lib/dll
	$(CXX) -shared -o lib/dll/libUtils.so $(OBJS) $(LIBS)

lib/lib/libUtils.a: $(OBJS)
	@$(MKDIR) lib
	@$(MKDIR) lib/lib
	$(AR) lib/lib/libUtils.a $(OBJS)
