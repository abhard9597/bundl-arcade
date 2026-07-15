CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 `wx-config --cxxflags`
LDFLAGS = `wx-config --libs` `wx-config --libs media`

SRC = main.cpp AnagramsMainFrame.cpp MenuPanel.cpp GamePanel.cpp GameLogic.cpp PauseDialog.cpp Dictionary.cpp RoundOverDialog.cpp SoundManager.cpp StatsManager.cpp AppTheme.cpp
OBJ = $(SRC:.cpp=.o)

TARGET = AnagramsApp

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)

fast:
	$(MAKE) -j 8

.PHONY: run clean fast
