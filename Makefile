.PHONY: all

CXX = ppc-amigaos-g++
CC = ppc-amigaos-gcc
STRIP = ppc-amigaos-strip

OPTIMIZE = -O3 -mstrict-align
CXXFLAGS = -std=c++17 -gstabs -mcrt=clib4 -Wall -fpermissive $(OPTIMIZE)
CFLAGS = -gstabs -mcrt=clib4 -Wall $(OPTIMIZE)
LDFLAGS=-mcrt=clib4 -athread=native -lcurl -lbrotlidec -lbrotlicommon -lnghttp2 -lrtmp -lssl -lcrypto  -lpsl -lidn2 -lunistring -lpthread -larchive -llz4 -llzma -lbz2 -lz -lsqlite3 -latomic -lauto 

MKDIR = mkdir -p
CP = cp
RM = rm -fr

RELEASE_DIR = apt
OBJ=build/
SRC=src/
INCLUDE=include/
CPP_OBJS=$(patsubst src/%.cpp, build/%.o, $(wildcard $(SRC)*.cpp))
C_OBJS=$(patsubst src/%.c, build/%.o, $(wildcard $(SRC)*.c))

apt = apt

all: build_dir $(apt)

$(apt): $(CPP_OBJS) $(C_OBJS)
	$(CXX) $(CXXFLAGS) -o build/$(apt).debug $(CPP_OBJS) $(C_OBJS) $(LDFLAGS)
	$(STRIP) build/$(apt).debug -o build/$(apt)

$(OBJ)%.o : $(SRC)%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ)%.o : $(SRC)%.c
	$(CC) $(CFLAGS) -c $< -o $@

build_dir:
	$(MKDIR) build

clean:
	rm -f build/*

release: all
	$(MKDIR) $(RELEASE_DIR)
	$(CP) build/$(apt) $(RELEASE_DIR)/
	$(CP) README.md $(RELEASE_DIR)/
	$(CP) sources.list $(RELEASE_DIR)/
	$(CP) curl-ca-bundle.crt $(RELEASE_DIR)/
	$(CP) LICENSE-GPL $(RELEASE_DIR)/
	lha -ao5i apt.lha $(RELEASE_DIR)
	$(RM) $(RELEASE_DIR)