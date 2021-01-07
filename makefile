#  dir base
HEADER_DIR := include
SRC_DIR := src
OBJ_DIR := build/obj
BIN_DIR := build/bin
EXP_DIR := exp
EXP_BIN_DIR := $(EXP_DIR)/bin
EXP_OBJ_DIR := $(EXP_DIR)/obj
EXP_SRC_DIR := $(EXP_DIR)/src
TEST_DIR := test
TEST_SRC_DIR := $(TEST_DIR)/src
TEST_OBJ_DIR := $(TEST_DIR)/obj
TEST_BIN_DIR := $(TEST_DIR)/bin

#  set of all source files & object files
# SRC := $(wildcard $(SRC_DIR)/*.cc)
# OBJ := $(patsubst $(SRC_DIR)%,$(OBJ_DIR)%,$(SRC:.cc=.o))

#  start of the programe
TARGET := matching_main
all : $(TARGET)

#  compile utility $ compile options
CXX := g++
CXXFLAGS := -march=native -std=c++11 -o3 -pthread -I$(HEADER_DIR)#-mcmodel=medium


# $(OBJ_DIR)/write_worker.o : $(SRC_DIR)/write_worker.cc $(OBJ_DIR)/memory_pool.o /usr/lib/libisal.so
# 	$(CXX) $(CXXFLAGS) -c $<  -o $@

# $(OBJ_DIR)/tcp_node.o : $(OBJ_DIR)/tcp_server.o $(OBJ_DIR)/tcp_client.o
# 	$(CXX) $(CXXFLAGS) -c $<  -o $@

#  rule of compiling object with corresponding  source file
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cc
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXP_OBJ_DIR)/%.o : $(EXP_SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_OBJ_DIR)/%.o : $(TEST_SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

#  get main
$(TARGET): $(OBJ_DIR)/$(TARGET).o $(OBJ_DIR)/stripe.o $(OBJ_DIR)/matching_generator.o
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $(BIN_DIR)/$@

#  clean obj % bin
clean:
	rm -f $(BIN_DIR)/* $(OBJ_DIR)/* $(TEST_OBJ_DIR)/* $(TEST_BIN_DIR)/* $(EXP_OBJ_DIR)/* $(EXP_BIN_DIR)/*

tcpecho : $(TEST_OBJ_DIR)/tcpecho.o
	$(CXX) $(CXXFLAGS) $^ -o $(TEST_BIN_DIR)/$@ /usr/local/lib/libsockpp.so

tcpechosvr : $(TEST_OBJ_DIR)/tcpechosvr.o
	$(CXX) $(CXXFLAGS) $^ -o $(TEST_BIN_DIR)/$@ /usr/local/lib/libsockpp.so

sockpp_test : tcpecho tcpechosvr

tcp_test : tcp_server_test tcp_client_test

tcp_server_test : $(TEST_OBJ_DIR)/tcp_server_test.o $(OBJ_DIR)/tcp_server.o /usr/local/lib/libsockpp.so $(OBJ_DIR)/memory_pool.o $(OBJ_DIR)/write_worker.o $(OBJ_DIR)/compute_worker.o /usr/lib/libisal.so
	$(CXX) $(CXXFLAGS) $^ -o $(TEST_BIN_DIR)/$@

tcp_client_test : $(TEST_OBJ_DIR)/tcp_client_test.o $(OBJ_DIR)/tcp_client.o /usr/local/lib/libsockpp.so
	$(CXX) $(CXXFLAGS) $^ -o $(TEST_BIN_DIR)/$@

thread_pool_test : $(TEST_OBJ_DIR)/thread_pool_test.o $(OBJ_DIR)/memory_pool.o $(OBJ_DIR)/write_worker.o $(OBJ_DIR)/compute_worker.o
	$(CXX) $(CXXFLAGS) $^ -o $(TEST_BIN_DIR)/$@

isa-l_test : $(TEST_OBJ_DIR)/isa-l_test.o /usr/lib/libisal.so
	$(CXX) $(CXXFLAGS) $^ -o $(TEST_BIN_DIR)/$@

node_main : $(OBJ_DIR)/node_main.o $(OBJ_DIR)/tcp_node.o $(OBJ_DIR)/tcp_server.o $(OBJ_DIR)/tcp_client.o /usr/local/lib/libsockpp.so $(OBJ_DIR)/memory_pool.o $(OBJ_DIR)/write_worker.o $(OBJ_DIR)/compute_worker.o /usr/lib/libisal.so $(OBJ_DIR)/ncscale_simulation.o  $(OBJ_DIR)/stripe.o $(OBJ_DIR)/matching_generator.o
	$(CXX) $(CXXFLAGS) $^ -o $(BIN_DIR)/$@

ncscale_test : $(TEST_OBJ_DIR)/ncscale_test.o $(OBJ_DIR)/ncscale_simulation.o
	$(CXX) $(CXXFLAGS) $^ -o $(TEST_BIN_DIR)/$@

matching_exp: $(EXP_OBJ_DIR)/matching_exp.o $(OBJ_DIR)/stripe.o $(OBJ_DIR)/matching_generator.o
	$(CXX) $(CXXFLAGS) $^ -o $(EXP_BIN_DIR)/$@