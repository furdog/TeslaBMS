#!/bin/bash

# Set environment variables
if [ -z "${GIT_REPO_VERSION+x}" ]; then
	export GIT_REPO_VERSION=$(git describe --tags)
fi

# 2. Get the branch (e.g., test/name)
raw_branch=$(git rev-parse --abbrev-ref HEAD)

# 3. Replace / with - and export (e.g., test-name)
export GIT_REPO_BRANCH=${raw_branch//\//-}

# 4. Combine them into your final name
export FULL_VERSION="${GIT_REPO_VERSION}-${GIT_REPO_BRANCH}"

echo $FULL_VERSION

###############################################################################
# CONFIGURATION:
###############################################################################
COMPILER=arduino-cli

SERIAL_PORT=COM113
MONITOR_BAUD=115200
export OTA_IP="7.7.7.7"

export PROJECT="TeslaBOX"
export HARDWARE=esp32

#EXTRA_FLAGS="-v"

###############################################################################
# TARGETS AVAILABLE
###############################################################################
export DEVICE_ID=$PROJECT"_"$HARDWARE"_"$FULL_VERSION

if [ "$HARDWARE" == "esp32" ]; then
	BOARD=esp32:esp32:nodemcu-32s

	PROPS='--build-property build.partitions=partitions
	       --build-property upload.maximum_size=1638400'
	echo "PROPS: " ${PROPS}

	#echo "#define HARDWARE $HARDWARE" > target.gen.h
	#echo "#define DEVICE_ID \"$DEVICE_ID\"" >> target.gen.h
else
	echo "Bad target!"
	exit 1
fi

# Function to compile and upload
compile() {
	#Побудувати веб інтерфейс
	#echo "Building web..."
	#cd libraries/lwDASH-main
	#export GIT_WEB_VERSION=$(git describe --tags)
	#bash make.sh ../../web_interface.js

	#cd ../../
	#input_file="libraries/lwDASH-main/build/index.h"
	#output_file="web_interface.h"

	#Копіюємо згенерований gz массив
	#cp -u "$input_file" "$output_file"
	#sed -i 's/unsigned char/const PROGMEM char/g' "$output_file"

	#echo "HTML content has been written to $output_file"

	cp -u libraries/TeslaBMS-main/tesla_bms.h ./
	while ! ${COMPILER} compile -b ${BOARD} --warnings "all" ${PROPS} -e --libraries "libraries/"; do
		read -p "Press any key to continue "
		exit
	done

	#Create tagged binary
	local BOARD_PATH="${BOARD//:/\.}"  # Replace ':' with '.'
	local SRC="build/${BOARD_PATH}/$(basename "$PWD").ino.bin"
	local DST="build/${BOARD_PATH}/$DEVICE_ID.bin"
	echo "cp $SRC $DST"
	cp $SRC $DST
}

upload() {
	if [ -n "${AUTOFLASH_PORT+x}" ]; then
		while ! ${COMPILER} upload -b ${BOARD}${UPLOADSPEED} -p ${AUTOFLASH_PORT}; do
			sleep 1
		done
	fi
}

# Function to monitor
monitor() {
	if [ -n "${MONITOR_PORT+x}" ]; then
		while true; do
			${COMPILER} monitor -p ${MONITOR_PORT} --config baudrate=${MONITOR_BAUD};
			sleep 1
		done
	fi
}

erase() {
	if [ -n "${MONITOR_PORT+x}" ]; then
		while true; do
			 ${COMPILER} upload -b ${BOARD}${UPLOADSPEED} -p ${AUTOFLASH_PORT} --erase;
			sleep 1
		done
	fi
}

# Check if the argument is "monitor"
if [ "$1" == "monitor" ]; then
	monitor
elif [ "$1" == "erase" ]; then
	erase
elif [ "$1" == "upload" ]; then
	upload
elif [ "$1" == "setup" ]; then
	./setup.sh
elif [ "$1" == "clean" ]; then
	rm -rf build
	rm -rf libraries
	rm -rf tools
else
	compile
	upload
	monitor
fi

read -p "Press any key to continue "
