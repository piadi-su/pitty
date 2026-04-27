CC = gcc

CFLAGS = `pkg-config --cflags gtk+-3.0 vte-2.91`
LIBS   = `pkg-config --libs gtk+-3.0 vte-2.91`

SRC = src/main.c
OUT_DIR = bin
TARGET = $(OUT_DIR)/pitty

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p $(OUT_DIR)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LIBS)

clean:
	rm -rf $(OUT_DIR)

run: all
	./$(TARGET)
