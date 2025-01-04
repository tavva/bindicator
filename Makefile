CLI?=/opt/homebrew/bin/arduino-cli
PORT?=/dev/cu.usbmodem2101
BOARD?=esp32:esp32:esp32s3
SKETCH=$(shell ls *.ino | head -n 1)
hr=bash -c 'COLS=`tput cols`;x=1;dots=""; while [ $$x -le $$COLS ]; do dots="$$dots""-"; x=$$(( $$x + 1 )); done; dots=$${dots:0:$$COLS}; echo $$dots;'

all: upload

compile:
	@${hr}
	@echo Building...
	@${hr}
	@${CLI} -v compile --fqbn $(BOARD) $(SKETCH)

nocache:
	@${hr}
	@echo Clean and build (no cache)
	@${hr}
	@${CLI} -v compile --clean --upload -p $(PORT) --fqbn $(BOARD) $(SKETCH)

upload:
	@${hr}
	@echo Building and uploading afterwards
	@${hr}
	@${CLI} -v compile --upload -p $(PORT) --fqbn $(BOARD) $(SKETCH)

update:
	@${hr}
	@echo Updating Arduino platform...
	@${hr}
	@${CLI} update
	@${CLI} upgrade

list:
	@${hr}
	@echo Listing all installed board's FQBN
	@${hr}
	@${CLI} board listall
