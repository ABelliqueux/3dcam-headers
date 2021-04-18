#include "pad.h"

void get_digital_direction( Controller_Data *c, int buttondata ) // get analog stick values
{
int i;

	i = ~(buttondata);

	if( i & 0x80 )
		c->xpos -= 1;
				
	if( i & 0x20 )
		c->xpos += 1;
				
	if( i & 0x40 )
		c->ypos += 1;
				
	if( i & 0x10 )
		c->ypos -= 1;
}

void read_controller( Controller_Data *c, unsigned char *buf, int port )  // get the raw values from controller
{
    register int mouse_x, mouse_y, x;
    register Gun_Position *g;

    c->status =  buf[0];    // Copy over raw controller data
    c->type =    buf[1];
    c->button1 = buf[2];
    c->button2 = buf[3];
    c->analog0 = buf[4];
    c->analog1 = buf[5];
    c->analog2 = buf[6];
    c->analog3 = buf[7];

    if( buf[0] == 0xff )    // If controller returns BAD status then bail on it.
    {
        c->type = 0;
        return;
    }

    // Look at the controller type code & process controller data as indicated

    switch( c->type )
    {
        case 0x12:      // Sony Mouse
            mouse_x = buf[4];
            mouse_y = buf[5];

            if( mouse_x & 0x80 )
                mouse_x |= 0xffffff80;
            if( mouse_y & 0x80 )
                mouse_y |= 0xffffff80;

            c->xpos += mouse_x;
            c->ypos += mouse_y;
            break;
            
        case 0x23:      // Namco negCon
                        // Steering wheel
                        // Sankyo Pachinko controler
            get_digital_direction( c, buf[2] );
            break;
        
        case 0x53:      // Analog 2-stick
            get_digital_direction( c, buf[2] );
            break;

        case 0x41:      // Standard Sony PAD controller
            get_digital_direction( c, buf[2] );
            break;

        default:        // If don't know what it is, treat it like standard controller
            get_digital_direction( c, buf[2] );
            break;
    }
}
