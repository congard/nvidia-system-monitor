#!/bin/bash

function checkFile() {
    if [ -f "$1" ]
    then
	    echo "$1 - found"
        found=1
    else
	    echo "$1 - not found"
        found=0
    fi
}

function checkDir() {
    if [ -d "$1" ]
    then
	    echo "$1 - found"
        found=1
    else
	    echo "$1 - not found"
        found=0
    fi
}

function error() {
    echo $1
    exit 0
}

if [ "$EUID" -ne 0 ]
    then error "You must execute this file via root (sudo ./install.sh)"
fi

checkDir /usr/include/wx-3.0
if [ $found == 0 ]
    then error "Can't find wx-3.0 include dir. Have you installed wxWidgets (wxgtk3 and wxgtk packages, if you are using ArchLinux)?"
fi

checkDir /usr/lib/wx
if [ $found == 0 ]
    then error "Can't find wx lib dir. Have you installed wxWidgets (wxgtk3 and wxgtk packages, if you are using ArchLinux)?"
fi

checkFile /bin/wx-config
if [ $found == 0 ]
then
    checkFile /usr/bin/wx-config
    if [ $found == 0 ]
    then
        error "Can't find wx-config. Have you installed wxWidgets (wxgtk3 and wxgtk packages, if you are using ArchLinux)?"
    fi
fi

checkFile /bin/g++
if [ $found == 0 ]
then
    checkFile /usr/bin/g++
    if [ $found == 0 ]
    then
        error "Can't find g++ compiler"
    fi
fi

echo "All right"
echo "Executing make.sh"

./make.sh install-build

echo

checkFile bin/nvidia-system-monitor
if [ $found == 0 ]
then
    error "Compilation for some reason was failed"
fi

echo "Compilation done"

echo "Installation begins"

mkdir -p /usr/share/doc/nvidia-system-monitor
mkdir -p /usr/share/icons/hicolor/512x512/apps
cp bin/nvidia-system-monitor /usr/bin/nvidia-system-monitor
cp nvidia-system-monitor.desktop /usr/share/applications/nvidia-system-monitor.desktop
cp -r src/html /usr/share/doc/nvidia-system-monitor
cp icon.png /usr/share/icons/hicolor/512x512/apps/nvidia-system-monitor.png

echo "Installation done"