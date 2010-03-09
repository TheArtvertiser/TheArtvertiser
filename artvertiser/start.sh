#!/bin/sh

# say we were started
touch start.sh.started

# avoid shell interference (see info coreutils 'sleep invocation')
echo start.sh sleeping 5 seconds
env sleep 5
/home/artvertiser/Desktop/working/The-Artvertiser/artvertiser/artvertiser -vd 0 -vs 640 480 -ml models.xml 2>&1 /home/artvertiser/Desktop/working/The-Artvertiser/artvertiser/artvertiser_via_start.sh.log

# say we finished
touch start.sh.finished

