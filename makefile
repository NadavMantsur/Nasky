DS=a
DS1=nasky_test
DS2=nasky
DS3=logger
DS4=thread_pool


SRC_DIR := ./src
INC_DIRS := ./include
FRAMEWORK_INC_DIRS := ./framework/include

# Add a prefix to INC_DIRS. So './include' would become '-iquote./include'. 
INC_FLAG := $(addprefix -iquote,$(INC_DIRS))
FRAMEWORK_INC_FLAG := $(addprefix -iquote,$(FRAMEWORK_INC_DIRS))

CC=g++ -std=c++17
CPPFLAGS= $(INC_FLAG) $(FRAMEWORK_INC_FLAG) -pedantic-errors -Wall -Wextra -g -lm -pthread

$(DS).out: $(DS1).o $(DS2).o $(DS3).o $(DS4).o
	$(CC) $(CPPFLAGS) $^ -o $@
	
$(DS1).o: ./test/$(DS1).cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

$(DS2).o: ./src/$(DS2).cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

$(DS3).o: ./framework/src/$(DS3).cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

$(DS4).o: ./framework/src/$(DS4).cpp
	$(CC) $(CPPFLAGS) -c $< -o $@



.PHONY: vlg
vlg: 
	valgrind --leak-check=yes --track-origins=yes ./$(DS).out
	
.PHONY: clean
clean: 
	-rm -f *.out
	-rm -f *.o

