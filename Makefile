CC := g++
PROJ := cchat
C_FLAGS := -std=c++20 -Wall -Wextra -Wpedantic

SRC_DIR := src
BLD_DIR := build
INS_DIR := /usr/bin
DOX_DIR := docs/doxygen

H_DEPS := args.hpp utility.hpp storage.hpp logger.hpp connect.hpp entity.hpp message.hpp session.hpp
H_REFS := $(addprefix $(SRC_DIR)/, $(H_REFS))

C_DEPS := args.cpp utility.cpp storage.cpp message.cpp connect.cpp client_gui.cpp client_session.cpp client_entity.cpp
C_OBJS := $(addprefix $(BLD_DIR)/, $(C_DEPS:%.cpp=%.o))

S_DEPS := args.cpp utility.cpp storage.cpp message.cpp connect.cpp server_session.cpp server_entity.cpp
S_OBJS := $(addprefix $(BLD_DIR)/, $(S_DEPS:%.cpp=%.o))

.PHONY: all docs install clean

all: client server

client: folders $(C_OBJS)
	$(CC) $(C_FLAGS) -o $(BLD_DIR)/$(PROJ)-client $(SRC_DIR)/client.cpp $(C_OBJS) -lncurses

server: folders $(S_OBJS)
	$(CC) $(C_FLAGS) -o $(BLD_DIR)/$(PROJ)-server $(SRC_DIR)/server.cpp $(S_OBJS)

$(BLD_DIR)/%.o: $(SRC_DIR)/%.cpp $(H_REFS)
	$(CC) $(C_FLAGS) -c -o $@ $<

install: install-client install-server

install-client: client
	sudo cp $(BLD_DIR)/$(PROJ)-client $(INS_DIR)/$(PROJ)-client

install-server: server
	sudo cp $(BLD_DIR)/$(PROJ)-server $(INS_DIR)/$(PROJ)-server

uninstall:
	sudo rm $(INS_DIR)/$(PROJ)-client $(INS_DIR)/$(PROJ)-server

docs: folders
	doxygen

folders:
	mkdir -p $(BLD_DIR) $(DOX_DIR)

clean:
	rm -rf $(BLD_DIR) $(DOX_DIR)
