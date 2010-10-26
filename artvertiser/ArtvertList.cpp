/*
 *  ArtvertList.cpp
 *  artvertiser
 *
 *  Created by damian on 26/10/10.
 *  Copyright 2010 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "ArtvertList.h"

void ArtvertList::saveToXml( string path )
{	
	if ( !needs_saving )
		return;
	needs_saving = false;
	if ( path.size() == 0 )
		return;
	
	lock();
	
	ofxXmlSettings data;
	data.addTag( "artverts" );
	data.pushTag( "artverts" );
	// work out which artverts belong with which
	map< string, vector<int> > artverts_grouped_by_model;
	for ( int i=0; i<artvert_list.size(); i++ )
	{
		artverts_grouped_by_model[artvert_list[i]->getModelFile()].push_back( i );
	}
	// preserve the existing order
	for ( int i=0; i<artvert_list.size(); i++ )
	{
		vector<int>& siblings = artverts_grouped_by_model[ artvert_list[i]->getModelFile() ];
		int index = data.addTag("advert");
		data.pushTag("advert", index );
		for ( int j=0; j<siblings.size(); ++j )
		{
			Artvert::saveArtvertToXml( data, artvert_list[siblings[j]], /*first? then save model file info*/(j==0) );
		}
		data.popTag();
	}
	data.popTag();
	data.saveFile( path );
	
	unlock();
}


vector<string> ArtvertList::getDescriptions()
{
	vector<string> return_me;
	lock();
	for ( int i=0; i<artvert_list.size(); i++ )
	{
		return_me.push_back( artvert_list[i]->getDescription() );
	}
	unlock();
	return return_me;
}
