.PHONY: all

CXX = ppc-amigaos-g++
CXXFLAGS = -std=c++11 -gstabs -mcrt=clib4 -Wall
LDFLAGS=-mcrt=clib4 -athread=native -lcurl -lssl -lcrypto -lbrotlidec -lbrotlicommon -lnghttp2 -lidn2 -lunistring -lpthread -lz -lauto

OBJ=build/
SRC=src/
INCLUDE=include/
OBJS=$(patsubst src/%.cpp, build/%.o, $(wildcard $(SRC)*.cpp))
$(info $(OBJ))

apt = apt

all: build_dir $(apt)

$(apt): $(OBJS)
	$(CXX) $(CXXFLAGS) -o build/$(apt) $(OBJS) $(LDFLAGS)

$(OBJ)%.o : $(SRC)%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

build_dir:
	mkdir -p build

clean:
	rm -f build/*