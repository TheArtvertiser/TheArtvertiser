#!/usr/bin/python

import pprint
import sys
import os.path

import xml.dom.minidom
from xml.dom.minidom import Node

def usage():
	print "usage:"
	print sys.argv[0]+" [-m] <models.xml>"
	print " -m: on exit, print all paths to artvert movies found in xml"


# parse args
models_list = ""
dump_movies = 0
movies = []
if len(sys.argv) == 1:
	usage()
	exit(1)
if sys.argv[1] == "-m":
	if len(sys.argv) == 2:
		usage()
		exit(1)
	models_list = sys.argv[2]
	dump_movies = 1
else:
	if len(sys.argv) != 2:
		usage()
		exit(1)
	models_list = sys.argv[1]

# parse xml
if not os.path.exists(models_list):
	print >> sys.stderr, "couldn't load xml models list '"+models_list+"'"
	usage()
	exit(1)
try:
	doc = xml.dom.minidom.parse(models_list)
except xml.parsers.expat.ExpatError as error:
	print >> sys.stderr, "error while parsing xml models list: '"+str(error)+"'"
	exit(1)

# go thorugh nodes
for advert in doc.childNodes[0].childNodes:
	if advert.nodeType != Node.ELEMENT_NODE:
		continue

	# get name for the advert
	advert_name = ""
	for child in advert.childNodes:
		if child.nodeName == "name":
			advert_name = child.childNodes[0].nodeValue
		elif child.nodeName == "advert":
			advert_name = child.childNodes[0].nodeValue

	if advert_name == "":
		print >> sys.stderr, "missing <name> or <advert> node near '"+advert.toxml()+"'"
		advert_name = "<<missing>>"

	# get model filename
	model_filename_nodes = advert.getElementsByTagName("model_filename")
	if model_filename_nodes.length < 1:
		print >> sys.stderr, "advert '"+advert_name+"': xml missing model_filename node"
	else:
		model_filename = model_filename_nodes[0].childNodes[0].nodeValue
		if not os.path.exists(model_filename):
			print >> sys.stderr, "advert '"+advert_name+"': can't find model file "+model_filename+"'"
		if not os.path.exists(model_filename+".artvertroi"):
			print >> sys.stderr, "advert '"+advert_name+"': can't find artvertroi file "+model_filename+".artvertroi'"
		if not os.path.exists(model_filename+".roi"):
			print >> sys.stderr, "advert '"+advert_name+"': can't find roi file "+model_filename+".roi'"
		if not os.path.exists(model_filename+".classifier"):
			print >> sys.stderr, "advert '"+advert_name+"': can't find classifier directory "+model_filename+".classifier'"



	# loop through artvert nodes
	for artvert in advert.getElementsByTagName("artvert"):
		title = ""
		artist = ""
		image_filename = ""
		movie_filename = ""

		for child in artvert.childNodes:
			if child.nodeType != Node.ELEMENT_NODE:
				continue
			if child.nodeName == "title":
				title = child.childNodes[0].nodeValue
			elif child.nodeName == "name":
				title = child.childNodes[0].nodeValue
			elif child.nodeName == "artist":
				artist = child.childNodes[0].nodeValue
			elif child.nodeName == "image_filename":
				image_filename = child.childNodes[0].nodeValue
			elif child.nodeName == "movie_filename":
				movie_filename = child.childNodes[0].nodeValue
			else:
				print >> sys.stderr, "advert '"+advert_name+"': unrecognized node '"+child.nodeName+"' near '"+artvert.toxml()

		
		# check title and artist nodes present
		if title == "" or artist == "":
			print >> sys.stderr, "advert '"+advert_name+"': artvert missing title or artist node near '"+artvert.toxml()+"'"

		# check existence of _filename nodes
		if image_filename == "" and movie_filename == "":
			print >> sys.stderr, "advert '"+advert_name+"': artvert '"+title+"' missing image_filename or movie_filename node near '"+artvert.toxml()+"'"
		if image_filename != "" and movie_filename != "":
			print >> sys.stderr, "advert '"+advert_name+"': artvert '"+title+"' has both image_filename and movie_filename, which should it be? near '"+artvert.toxml()+"'"

		# test for existence of image_filename/movie_filename
		if image_filename != "" :
			if not os.path.exists(image_filename):
				print >> sys.stderr, "advert '"+advert_name+"': artvert '"+title+"': couldn't find image file '"+image_filename+"'"
		elif movie_filename != "":
			if not os.path.exists(movie_filename):
				print >> sys.stderr, "advert '"+advert_name+"': artvert '"+title+"': couldn't find movie file '"+movie_filename+"'"
			movies.append( movie_filename )

if dump_movies:
	for movie in movies:
		print movie
