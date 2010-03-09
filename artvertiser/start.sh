#!/bin/sh

# say we were started
touch start.sh.started

# avoid shell interference (see info coreutils 'sleep invocation')
echo start.sh sleeping 10 seconds
env sleep 5
echo 5
env sleep 5
echo sleeped
echo starting artvertiser...
/home/artvertiser/Desktop/working/The-Artvertiser/artvertiser/artvertiser -vd 0 -vs 640 480 -ml models.xml -binoc 2>&1 | tee /home/artvertiser/Desktop/working/The-Artvertiser/artvertiser/artvertiser_via_start.sh.log


# say we finished
touch start.sh.finished

