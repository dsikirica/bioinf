RELEASE_DIR=build/release
SOURCES_DIR=src
OBJECT_DIR=build/object

CPP_FILES := $(wildcard $(SOURCES_DIR)/*.cpp)
OBJ_FILES := $(addprefix $(OBJECT_DIR)/,$(notdir $(CPP_FILES:.cpp=.o)))
LD_FLAGS =
CC = g++
CC_FLAGS = -Wall -I include/

all: $(RELEASE_DIR)/out

$(RELEASE_DIR)/out: $(OBJ_FILES)
	$(CC) $(LD_FLAGS) -o $@ $^

$(OBJECT_DIR)/%.o: $(SOURCES_DIR)/%.cpp
	$(CC) $(CC_FLAGS) -c -o $@ $<

clean:
	rm *.o $(OBJECT_DIR)/

run:
	./build/release/out

build_and_run: all run
