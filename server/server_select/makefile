CC:= g++
TARGET:= server
INCLUDE:= -I./
LIBS:= -lpthread

CXXFALGS:= -std=c++11 -g -Wformat -D_REENTRANT

OBJECTS:= server.o

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(LIBS)

%.o:%.cpp
	$(CC) -c $(CXXFLAGS) $(INCLUDE) $< -o $@

.PHONY : clean
clean:
	rm -f $(OBJECTS) $(TARGET)
