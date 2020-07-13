CXXFLAGS += -Wall -g

SRC_DIR = ./src
BIN_DIR = ./bin
INCLUDE_DIR = ./include

CXXFLAGS += -I$(INCLUDE_DIR)

all: directories analyzer

directories:
	mkdir -p $(BIN_DIR)

analyzer: $(SRC_DIR)/analyzer.cpp $(INCLUDE_DIR)/tables.h
	$(CXX) $(CXXFLAGS) $< -o $(BIN_DIR)/$@

clean:
	rm -rvf $(BIN_DIR)
