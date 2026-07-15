CXX = g++
CXXFLAGS = `wx-config --cxxflags` -std=c++17
LDFLAGS = `wx-config --libs std,media`

TARGET = bundl

SOURCES = main.cpp MainFrame.cpp MenuPanel.cpp SettingsPanel.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f *.o $(TARGET)
