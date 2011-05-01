//
//  openFileDialogOSX.cpp
//  artvertiser
//
//  Created by damian on 5/1/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "openFileDialogOSX.h"

#include <AppKit/AppKit.h>

string openFileDialogOSX()
{
    string result_filename;
    
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    NSOpenPanel *opanel = [NSOpenPanel openPanel];
	[opanel setAllowsMultipleSelection:NO];
	int returnCode = [opanel runModalForDirectory:nil file:nil types:nil];
    
	if(returnCode == NSOKButton) {
		NSArray *filenames = [opanel filenames];
		NSString *file = [filenames objectAtIndex:0];
        result_filename = [file UTF8String];
	}
    
    [pool drain];
    
    return result_filename;
}