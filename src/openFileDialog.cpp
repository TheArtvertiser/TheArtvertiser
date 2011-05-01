//
//  openFileDialog.cpp
//  artvertiser
//
//  Created by damian on 5/1/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "openFileDialog.h"
#include "openFileDialogOSX.h"
#include "ofMain.h"

string openFileDialog()
{
    
    string result_filename = "";
    
#if defined TARGET_WIN32
    
    char szFileName[MAX_PATH] = "";
    
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = 0;
    
	if(GetOpenFileName(&ofn)) {
        result_filename = szFileName;
	}
    
#elif defined ( TARGET_OSX )

    // defined in openFileDialogOSX.mm
    result_filename = openFileDialogOSX();
    
#elif defined ( TARGET_LINUX )

    GtkWidget *dialog;
    
    dialog = gtk_file_chooser_dialog_new ("Open File",
                                              NULL,
                                              GTK_FILE_CHOOSER_ACTION_OPEN,
                                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                              GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                              NULL);
    
   if(gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        result_filename = filename;
        g_free (filename);
    }
    gtk_widget_destroy(dialog);
    
#endif

    return result_filename;
    
}
