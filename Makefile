GCC 	 := gcc
GPP		 := g++
CXXFLAGS := -Wall -Wextra -Werror -std=c++23 -ggdb
TARGET   := aec

all: $(TARGET)

$(TARGET): aec.cpp
	$(GPP) $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(TARGET)
