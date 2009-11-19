./glc-play parkhaus.glc -o - -y 1 | mencoder - -ovc lavc  -lavcopts vcodec=mpeg4:vbitrate=2000 -fps 50 -o parkhaus.avi
