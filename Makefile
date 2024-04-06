CXX = ppc-amigaos-g++
CXXFLAGS = -std=c++11 -gstabs -mcrt=clib4 -Wall
LDFLAGS=-mcrt=clib4 -athread=native -lcurl -lssl -lcrypto -lbrotlidec -lbrotlicommon -lnghttp2 -lidn2 -lunistring -lpthread -lz -lauto

apt = apt
source_files = repositoryManager.cpp repository.cpp ar.cpp main.cpp

all: $(apt)

$(apt): $(source_files:.cpp=.o)
	$(CXX) $(CXXFLAGS) -o $(apt) $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(apt)