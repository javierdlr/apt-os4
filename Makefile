.PHONY: all

CXX = ppc-amigaos-g++
CC = ppc-amigaos-gcc
OPTIMIZE = -O3 -mstrict-align
CXXFLAGS = -std=c++17 -gstabs -mcrt=clib4 -Wall -fpermissive $(OPTIMIZE)
CFLAGS = -gstabs -mcrt=clib4 -Wall $(OPTIMIZE)
LDFLAGS=-mcrt=clib4 -athread=native -lcurl -lbrotlidec -lbrotlicommon -lnghttp2 -lrtmp -lssl -lcrypto  -lpsl -lidn2 -lunistring -lpthread -larchive -llz4 -llzma -lbz2 -lz -lsqlite3 -latomic -lauto 

OBJ=build/
SRC=src/
INCLUDE=include/
CPP_OBJS=$(patsubst src/%.cpp, build/%.o, $(wildcard $(SRC)*.cpp))
C_OBJS=$(patsubst src/%.c, build/%.o, $(wildcard $(SRC)*.c))

apt = apt

all: build_dir $(apt)

$(apt): $(CPP_OBJS) $(C_OBJS)
	$(CXX) $(CXXFLAGS) -o build/$(apt) $(CPP_OBJS) $(C_OBJS) $(LDFLAGS)

$(OBJ)%.o : $(SRC)%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ)%.o : $(SRC)%.c
	$(CC) $(CFLAGS) -c $< -o $@

build_dir:
	mkdir -p build

clean:
	rm -f build/*