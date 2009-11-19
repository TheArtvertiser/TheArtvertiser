#!/bin/sh

rmmod uvcvideo
modprobe uvcvideo
uvcdynctrl -d /dev/video1 -i /etc/udev/data/046d/logitech.xml
uvcdynctrl -d /dev/video1 -s "Exposure, Auto" 1
uvcdynctrl -d /dev/video1 -s "Exposure (Absolute)" 300 

