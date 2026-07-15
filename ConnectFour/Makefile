CXX = g++
CXXFLAGS = -std=c++17 `wx-config --cxxflags` -MMD -MP
LDFLAGS = `wx-config --libs`

TARGET = ConnectFourApp

SRC = ConnectFourMain.cpp \
      ConnectFourPanel.cpp \
      ConnectFourGame.cpp \
      ConnectFourBoard.cpp \
      ConnectFourMenu.cpp \
      ConnectFourAI.cpp \
      SoundManager.cpp \
      ConnectFourStatsManager.cpp \
      ConnectFourStatsDialog.cpp \
      ConnectFourAppTheme.cpp

OBJECTS = $(SRC:.cpp=.o)
DEPS = $(SRC:.cpp=.d)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(DEPS) $(TARGET)

-include $(DEPS)

.PHONY: all clean