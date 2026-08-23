CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -pthread

SRC = src

all: server client

server:
	$(CXX) $(CXXFLAGS) $(SRC)/server.cpp -o $(SRC)/server

client:
	$(CXX) $(CXXFLAGS) $(SRC)/client.cpp -o $(SRC)/client

clean:
	rm -f $(SRC)/server $(SRC)/client
