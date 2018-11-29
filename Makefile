CXX = cc
CXX_FLAGS = -pedantic -g

SRC_DIR = src

EXEC = ./client.out ./server.out

.PHONY: all
all: $(EXEC)

%.out: $(SRC_DIR)/%.c
	$(CXX) $(CXX_FLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -f *.out
 
