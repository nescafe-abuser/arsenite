CXX = g++
CXXFLAGS = -Wall -Wextra -Wswitch -ggdb -I./lib -I./

TARGET = arc
BUILD_DIR = build
SRC_DIR = src
LIB_DIR = lib

OBJS = $(BUILD_DIR)/compiler.o \
       $(BUILD_DIR)/lexer.o \
       $(BUILD_DIR)/parser.o \
       $(BUILD_DIR)/codegen.o

all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(CXXFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/compiler.o: $(SRC_DIR)/compiler.cpp $(LIB_DIR)/lexer.h $(LIB_DIR)/parser.hpp $(LIB_DIR)/codegen.hpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(BUILD_DIR)/lexer.o: $(LIB_DIR)/lexer.cpp $(LIB_DIR)/lexer.h
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(BUILD_DIR)/parser.o: $(LIB_DIR)/parser.cpp $(LIB_DIR)/parser.hpp $(LIB_DIR)/lexer.h
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(BUILD_DIR)/codegen.o: $(LIB_DIR)/codegen.cpp $(LIB_DIR)/codegen.hpp $(LIB_DIR)/parser.hpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

clean:
	rm -rf $(BUILD_DIR)

install: all
	cp $(BUILD_DIR)/$(TARGET) /usr/bin/$(TARGET)

uninstall:
	rm /usr/bin/$(TARGET)

.PHONY: clean all install uninstall
