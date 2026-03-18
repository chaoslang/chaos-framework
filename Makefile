CXX := clang++
CXXFLAGS := $(shell llvm-config --cxxflags) -fexceptions
LDFLAGS := $(shell llvm-config --ldflags --system-libs --libs core)

CHAOSC := chaosc
SRC := chaosc.cpp

EXAMPLES := $(wildcard examples/*.ch)
OUTPUTS := $(patsubst examples/%.ch,build/%,$(EXAMPLES))

all: $(CHAOSC)

$(CHAOSC): $(SRC)
	$(CXX) -o $@ $(SRC)

examples: $(CHAOSC) build $(OUTPUTS)

build:
	mkdir -p build

build/%: examples/%.ch $(CHAOSC) | build
	./$(CHAOSC) $< $@ exe

clean:
	rm -rf build $(CHAOSC)

.PHONY: all examples clean build
