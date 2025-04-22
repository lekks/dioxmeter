PLATFORM=arduino:avr:uno
PORT=/dev/ttyUSB0

.PHONY: all clean compile upload


all: compile

compile:
	arduino-cli compile -b $(PLATFORM)

upload: compile
	arduino-cli upload -b $(PLATFORM) -p $(PORT)

clean:
	rm -rf build
	
