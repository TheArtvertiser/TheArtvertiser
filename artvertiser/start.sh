#!/bin/sh

# say we were started
touch start.sh.started

# avoid shell interference (see info coreutils 'sleep invocation')

# manual fixed exposure
#echo setting camera properties exposure 500.0
#/home/artvertiser/Desktop/tools/unicap-0.9.7/examples/c/device_property/device_property -p "Exposure, Auto" -v 1.0
#/home/artvertiser/Desktop/tools/unicap-0.9.7/examples/c/device_property/device_property -p "Exposure (Absolute)" -v 500.0

# auto exposure
echo setting camera to auto-exposure
/home/artvertiser/Desktop/tools/unicap-0.9.7/examples/c/device_property/device_property -p "Exposure, Auto" -v 3.0

break
echo start.sh sleeping 20 seconds
env sleep 5
echo 5
env sleep 5
echo 10
env sleep 5
echo 15
env sleep 5
echo 20
#env sleep 5
#echo sleeped 25
echo starting artvertiser...
/home/artvertiser/Desktop/working/The-Artvertiser/artvertiser/artvertiser -vd 0 -vs 640 480 -ml models.xml -binoc $1 $2 $3 $4 $5 $6 $7 $8 $9 2>&1 | tee /home/artvertiser/Desktop/working/The-Artvertiser/artvertiser/artvertiser_via_start.sh.log
#./start_no_delay.sh

# say we finished
touch start.sh.finished

