#mencoder -nosound tv:// -tv driver=v4l2:device=$1:input=1:norm=0 -vf-add pp=lb -ovc raw -oac copy -o $2 

mencoder -cache 256 -nosound tv:// -tv driver=v4l2:device=$1:input=1:norm=0 -vf-add scale=720:576,pp=lb,format=yuy2 -ovc raw -oac copy -of rawvideo -o $2
