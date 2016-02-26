#!/bin/bash

build() {
	# Switch to Plugins folder
	cd Plugins

	# Build Default Plugin Set
	cd Default
	qmake && make
	cd ..

	# Build MarkDown Plugin
	cd MarkDown
	qmake && make
	cd ..

	# Switch to current directory
	cd ..
}

build5() {
	# Switch to Plugins folder
	cd Plugins

	# Build Default Plugin Set
	cd Default
	qmake-qt5 && make
	cd ..

	# Build MarkDown Plugin
	cd MarkDown
	qmake-qt5 && make
	cd ..

	# Switch to current directory
	cd ..
}

clean() {
	# Switch to Plugins folder
	cd Plugins

	# Build Default Plugin Set
	cd Default
	make clean
	cd ..

	# Build MarkDown Plugin
	cd MarkDown
	make clean
	cd ..

	# Switch to current directory
	cd ..
}

distclean() {
	# Switch to Plugins folder
	cd Plugins

	# Build Default Plugin Set
	cd Default
	make distclean
	cd ..

	# Build MarkDown Plugin
	cd MarkDown
	make distclean
	cd ..

	# Switch to current directory
	cd ..
}

install() {
	# First make sure we have built the libs
	build

	# Switch to Plugins folder
	cd Plugins

	# Build Default Plugin Set
	cd Default
	make install
	cd ..

	# Build MarkDown Plugin
	cd MarkDown
	make install
	cd ..

	# Switch to current directory
	cd ..
}

installLocal() {
	# First make sure we have built the libs
	build

	# Switch to Plugins folder
	cd Plugins

	# Build Default Plugin Set
	cd Default
	cp -f libDefaultPeekPlugins.so ~/.config/NewBreeze/plugins/ 2> /dev/null
	cp -f libDefaultPeekPlugins5.so ~/.config/NewBreeze/plugins5/ 2> /dev/null
	cd ..

	# Build MarkDown Plugin
	cd MarkDown
	cp -f libMarkDownPreview.so ~/.config/NewBreeze/plugins/ 2> /dev/null
	cp -f libMarkDownPreview5.so ~/.config/NewBreeze/plugins5/ 2> /dev/null
	cd ..

	# Switch to current directory
	cd ..
}

if [ "$1" == "build" ]; then
	build

elif [ "$1" == "clean" ]; then
	clean

elif [ "$1" == "distclean" ]; then
	distclean

elif [ "$1" == "install" ]; then
	if [ "$(id -u)" != "0" ]; then
		echo "[Error]: You need to run install as root."
		read -n 1 -s
		exit 1
	fi
	install

elif [ "$1" == "local" ]; then
	installLocal

else
	echo "BuildPlugins.sh - NewBreeze Plugins Builder"
	echo
	echo "Usage:"
	echo "    ./BuildPlugins.sh build          # Build Plugins"
	echo "    ./BuildPlugins.sh clean          # Do 'make clean' recursively for all plugins"
	echo "    ./BuildPlugins.sh distclean      # Do 'make distclean' recursively for all plugins"
	echo "    ./BuildPlugins.sh install        # Install Plugins globally: to /usr/share/newbreeze/plugins"
	echo "    ./BuildPlugins.sh local          # Install Plugins locally: to /usr/share/newbreeze/plugins"

fi