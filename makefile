CPP = g++
CFLAGS  = -g -Wall

TARGET = ts_to_xml

all: $(TARGET)
$(TARGET):
	$(CPP) $(CFLAGS) -o $(TARGET) *.cpp

clean:
	rm $(TARGET)