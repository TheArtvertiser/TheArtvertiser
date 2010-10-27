const int pin_r = 12;
const int pin_g = 10;
const int pin_b = 8;

boolean on_r = false;
boolean on_g = false;
boolean on_b = false;

void setup()
{
  pinMode( pin_r, INPUT );
  pinMode( pin_g, INPUT );
  pinMode( pin_b, INPUT );
  
  Serial.begin( 9600 );
}


void loop()
{
  boolean r = (HIGH==digitalRead( pin_r ));
  boolean g = (HIGH==digitalRead( pin_g ));
  boolean b = (HIGH==digitalRead( pin_b ));

  // check for change  
  if ( r != on_r || g != on_g || b != on_b )
  {
    Serial.print( r?'1':'0' );
    Serial.print( g?'1':'0' );
    Serial.print( b?'1':'0' );
    Serial.print( '\n' );
    on_r = r;
    on_g = g;
    on_b = b;
  }
}


