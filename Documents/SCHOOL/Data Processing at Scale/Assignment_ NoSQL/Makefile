# You may need to moify the following variables to match your environment.
# ROCKSDB_DIR = /home/username/rocksdb-7.10.2
ROCKSDB_DIR = /Users/iametp/rocksdb-7.10.2
HOMEBREW_PREFIX = /opt/homebrew

CC = g++
CFLAGS = -std=c++17 -Wall -Wextra
LDFLAGS = -L$(ROCKSDB_DIR) -L$(HOMEBREW_PREFIX)/lib -lrocksdb -lpthread -lz -lbz2 -llz4 -lzstd
CXXFLAGS = -I$(ROCKSDB_DIR)/include

.PHONY: clean 

all: tester

tester: tester.cc assignment-nosql.cc assignment-nosql.hpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) assignment-nosql.cc tester.cc $(LDFLAGS) -o tester 

clean:
	rm -f assignment-nosql tester

