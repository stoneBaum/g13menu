#!/bin/bash

mkdir -p build
g++ -o build/g13menu g13menu.cpp

echo "Adding system service"
sudo cp systemd/g13-menu.service /etc/systemd/system/g13-menu.service
sudo cp 92-g13menu.rules /usr/lib/udev/rules.d/92-g13menu.rules

echo "'installing' g13d and g13menu"
sudo cp -f build/g13menu /usr/bin/g13menu

echo "copying profiles to /etc/g13/profiles"
sudo cp -R -n profiles /etc/g13/
sudo cp -R -n profiles/menu/menuBind.bind /etc/g13/default.bind
