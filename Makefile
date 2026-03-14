CXX = g++
CXXFLAGS = -Wall -Wextra -Wswitch -ggdb -I./ 

TARGET = arc
OBJS = compiler.o lexer.o parser.o codegen.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(CXXFLAGS)

compiler.o: compiler.cpp lexer.h parser.hpp codegen.hpp
	$(CXX) -c compiler.cpp $(CXXFLAGS)

lexer.o: lexer.cpp lexer.h
	$(CXX) -c lexer.cpp $(CXXFLAGS)

parser.o: parser.cpp parser.hpp lexer.h
	$(CXX) -c parser.cpp $(CXXFLAGS)

codegen.o: codegen.cpp codegen.hpp parser.hpp
	$(CXX) -c codegen.cpp $(CXXFLAGS)

clean:
	rm -rf $(TARGET) $(OBJS)

install: all
	cp $(TARGET) /usr/bin/$(TARGET)

uninstall:
	rm /usr/bin/$(TARGET)
.PHONY: clean all
