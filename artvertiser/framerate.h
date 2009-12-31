/*****************************************

 frames.h

 from http://intranet.cs.man.ac.uk/software/OpenGL/frames.txt

 General purpose frames per second counter for OpenGL/GLUT GNU/Linux
 programs. Displays "Frames per second: N" at an arbitrary position in
 the window. Saves and restores the app's modelview and projection matrices,
 colour, and lighting.

 Author: Toby Howard. toby@cs.man.ac.uk.
 Version 2.1, 1 June 2006

 ====================

 Usage: to add an on-screen frames per second counter to your program, save
 this file alongside your app as "frames.h", and add:

    #include "frames.h"

 immediately after all your app's other #includes; then bracket all the
 code in your display() function, before swapping buffers, with

   frameStart();

 and

   frameEnd(void *font, GLclampf r, GLclampf g, GLclampf b,
            float x, float y);

     font:    font to use, e.g., GLUT_BITMAP_HELVETICA_10
     r, g, b: RGB text colour
     x, y:    text position in window: range [0,0] (bottom left of window)
              to [1,1] (top right of window).

 ====================

 Example:

    void display(void) {
      glClear(GL_COLOR_BUFFER_BIT);

      frameStart();

      // all the graphics code

      frameEnd(GLUT_BITMAP_HELVETICA_10, 1.0, 1.0, 1.0, 0.05, 0.95);

      glutSwapBuffers();
    }
*****************************************/

#include <sys/time.h>
#include <stdio.h>

struct timeval frameStartTime, frameEndTime;
// for smoothed fps
float fps_accumulator = 0;


/// r, g, b, x, y [0..1]
void drawGlutString( const char* str, GLclampf r, GLclampf g, GLclampf b, GLfloat x, GLfloat y )
{
    void* font = GLUT_BITMAP_8_BY_13;
    const char* ch;
  GLint matrixMode;
  GLboolean lightingOn;
 lightingOn= glIsEnabled(GL_LIGHTING);        /* lighting on? */
 if (lightingOn) glDisable(GL_LIGHTING);

 glGetIntegerv(GL_MATRIX_MODE, &matrixMode);  /* matrix mode? */

 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
   glLoadIdentity();
   gluOrtho2D(0.0, 1.0, 0.0, 1.0);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
     glLoadIdentity();
     glPushAttrib(GL_COLOR_BUFFER_BIT);       /* save current colour */
       glColor3f(r, g, b);
       glRasterPos3f(x, y, 0.0);
       for(ch= str; *ch; ch++) {
           if ( *ch == '\n' )
           {
               y -= 0.03f;
               glRasterPos3f( x, y, 0.0 );
           }
           else
                glutBitmapCharacter(font, (int)*ch);
       }
     glPopAttrib();
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(matrixMode);
 if (lightingOn) glEnable(GL_LIGHTING);
}


void frameStart(void) {
  gettimeofday(&frameStartTime, NULL);
}

void frameEnd( GLclampf r, GLclampf g, GLclampf b,
              GLfloat x, GLfloat y) {
  /* font: font to use, e.g., GLUT_BITMAP_HELVETICA_10
     r, g, b: text colour
     x, y: text position in window: range [0,0] (bottom left of window)
           to [1,1] (top right of window). */

  float elapsedTime;
  char str[30];

  gettimeofday(&frameEndTime, NULL);

  elapsedTime= frameEndTime.tv_sec - frameStartTime.tv_sec +
             ((frameEndTime.tv_usec - frameStartTime.tv_usec)/1.0E6);

  // cheap lpf smoothing
  fps_accumulator = (fps_accumulator*7 + 1.0f/elapsedTime) / 8;

  sprintf(str, "fps: %2.1f", fps_accumulator);
  //printf("%s\n",str);
  drawGlutString( str, r, g, b, x, y );

}

/* end of frames.h */
