CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -Wpedantic -std=c11 \
          -fstack-protector-strong \
          -D_FORTIFY_SOURCE=2 \
          -fPIE -pie
TARGET  = bxrd_prompt
SRC     = bxrd_prompt.c
DEST    = $(HOME)/.local/bin

.PHONY: all install clean

all: $(TARGET)

$(TARGET): $(SRC)
        $(CC) $(CFLAGS) -o $(TARGET) $(SRC)

install: all
        mkdir -p $(DEST)
        install -m 755 $(TARGET) $(DEST)/$(TARGET)
        @echo "Installed to $(DEST)/$(TARGET)"

clean:
        rm -f $(TARGET)
