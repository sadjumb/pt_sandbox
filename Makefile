CXX=g++
CXXFLAGS=-std=c++17 -o3 -Wall -Wextra -Werror

SRC_FILE=main.cpp
NAME=main

.PHONY: all
all:
	$(CXX) $(CXXFLAGS) -o $(NAME) $(SRC_FILE)

.PHONY: clean
clean:
	rm $(NAME)