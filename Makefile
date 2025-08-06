CXX = g++-14
CXXFLAGS = -g -std=gnu++26 -I. -Isrc/ast -Isrc/ast/reserved -Isrc/input -Isrc/parser -Isrc/tokenizer -Isrc/type

SRCS := $(shell find src -name '*.cpp')
OBJS := $(SRCS:.cpp=.o)
TARGET = cpt

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	if [ -d build ]; then rm -rf build; fi

test: $(TARGET)
	./scripts/test-runner.sh

install: build
	cmake --install build

build:
	cmake -S . -B build
	cmake --build build