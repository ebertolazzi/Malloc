BITS    = x64
WARN    = -Wall
CC      = gcc $(WARN)
CXX     = g++ -std=c++11 -pthread $(WARN)
AR      = ar rcs
LIBSGCC = -lstdc++ -lm -pthread

ALL_LIBS = $(LIBS) -Llib/lib  -Llib/dll -lUtils_mingw_$(BITS).dll
# -lHSL_mingw_$(BITS)

override DEFS += -DMINGW

all_libs: lib/dll/libUtils_mingw_$(BITS).dll.a lib/lib/libUtils_mingw_$(BITS)_static.a

lib/dll/libUtils_mingw_$(BITS).dll.a: $(OBJS)
	@$(MKDIR) lib
	@$(MKDIR) lib/lib
	@$(MKDIR) lib/bin
	@$(MKDIR) lib/dll
	$(CXX) -shared -o lib/bin/libUtils_mingw_$(BITS).dll $(OBJS) \
	-Wl,--out-implib=lib/dll/libUtils_mingw_$(BITS).dll.a \
	-Wl,--export-all-symbols -Wl,--enable-auto-import -Wl,--no-whole-archive \
	$(LIBS)

lib/lib/libUtils_mingw_$(BITS)_static.a: $(OBJS)
	@$(MKDIR) lib
	$(AR) lib/lib/libUtils_mingw_$(BITS)_static.a $(OBJS)
