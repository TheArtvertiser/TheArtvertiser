#!/bin/sh
mplayer tv:// -tv driver=v4l2:device=$1:input=1:norm=0 -vf pp=lb 
