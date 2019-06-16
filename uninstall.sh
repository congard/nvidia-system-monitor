#!/bin/bash

function error() {
    echo $1
    exit 0
}

if [ "$EUID" -ne 0 ]
    then error "You must execute this file via root (sudo ./uninstall.sh)"
fi

echo "Uninstallation begin"

rm -rf /usr/share/doc/nvidia-system-monitor
rm /usr/bin/nvidia-system-monitor
rm /usr/share/applications/nvidia-system-monitor.desktop
rm /usr/share/icons/hicolor/512x512/apps/nvidia-system-monitor.png

echo "Done"