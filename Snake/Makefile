CXX = g++
CXXFLAGS = -std=c++17 `wx-config --cxxflags` -MMD -MP
LDFLAGS = `wx-config --libs`
MAKEFLAGS += -j$(shell sysctl -n hw.ncpu)

SRC = Snake_wx.cpp SnakeGame.cpp TypeSelect.cpp MainMenu.cpp Enemy.cpp PauseMenu.cpp Theme.cpp
OBJ = $(SRC:.cpp=.o)

TARGET = SnakeApp

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

-include $(OBJ:.o=.d)

clean:
	rm -f *.o *.d $(TARGET)