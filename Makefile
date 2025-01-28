CXX = C:/msys64/mingw64/bin/g++
CXXFLAGS = -std=c++20 -Iinclude -Iinclude/SDL2 -Wall -Wextra -O2
LDFLAGS = -Llib -lWs2_32 -lmingw32 -lSDL2_ttf -lSDL2main -lSDL2 -mwindows

SRC = src/main.cpp \
      src/Application.cpp \
      src/Menu.cpp \
      src/Game.cpp \
      src/Block.cpp \
      src/Grid.cpp \
      src/Button.cpp \
      src/Network.cpp \
      src/RoomView.cpp \
      src/RoomList.cpp \
      src/OnlineGame.cpp 

OBJ = $(SRC:.cpp=.o)
TARGET = tetris

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -force $(OBJ) $(TARGET)
