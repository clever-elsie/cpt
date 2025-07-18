CXX = g++-14
CXXFLAGS = -std=gnu++26 -I. -Isrc/calc -Isrc/calc/expr -Isrc/calc/expr/node -Isrc/calc/expr/node/reserved

SRCS := $(shell find . -name '*.cpp')
OBJS := $(SRCS:.cpp=.o)
TARGET = cpt

all: $(TARGET)

$(TARGET): $(SRCS) $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) 