MCU = mega2560
TARGET = m2560
AVRDUDE_PROGRAMMER = stk600
AVRDUDE_PORT = usb

all:
	ino build -m $(MCU)

install:
	avrdude -p $(TARGET) -c $(AVRDUDE_PROGRAMMER) -P $(AVRDUDE_PORT) -U flash:w:.build/$(MCU)/firmware.hex
