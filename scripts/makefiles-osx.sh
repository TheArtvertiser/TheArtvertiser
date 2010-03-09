#!/bin/sh

for f in `find . -name Makefile`; do 
	cp $f.osx $f; 
done

