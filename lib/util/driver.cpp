#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */

/* 
   http://stackoverflow.com/questions/27159322/rgb-values-of-the-colors-in-the-ansi-extended-colors-index-17-255

   The color range of a 256 color terminal consists of 4 parts, often 5,
       in which case you actually get 258 colors:

    - Color numbers 0 to 7 are the default terminal colors, the actual RGB
       value of which is not standardized and can often be configured.

    - Color numbers 8 to 15 are the "bright" colors. Most of the time these
       are a lighter shade of the the color with index - 8. They are also not
       standardized and can often be configured. Depending on terminal and
       shell, they are often used instead of or in conjunction with bold font
       faces.

    - Color numbers 16 to 231 are RGB colors. These 216 colors are defined by
       6 values on each of the three RGB axes. That is instead of values 0 -
       255, each color only ranges from 0 - 5.

       The color number is then calculated like this:

       number = 16 + 36 * r + 6 * g + b
       with r, g and b in the range 0 - 5.

    - The color numbers 232 to 255 are grayscale with 24 shades of gray from dark to light.

    - The default colors for foreground and background. In many terminals they
       can be configured independently from the 256 indexed colors, giving an
       additional two configurable colors . You get them when not setting any
       other color or disabling other colors (i.e. print '\e[m').
*/

/*
   In 256-color mode, the color-codes are the following:

   0x00-0x07:  standard colors (as in ESC [ 30–37 m)
   0x08-0x0F:  high intensity colors (as in ESC [ 90–97 m)
   0x10-0xE7:  6 × 6 × 6 = 216 colors: 16 + 36 × r + 6 × g + b (0 ? r, g, b ? 5)
   0xE8-0xFF:  grayscale from black to white in 24 steps

   Set the foreground color to index N: \033[38;5;${N}m
   Set the background color to index M: \033[48;5;${M}m
*/

int main( int argc, char **argv )
{
    int fg, bg, bold;

	// This line has a tab
		// Two tabs here

    printf( "\n\033[1;33m --- Terminal Color Chart ---\033[0m\n\n" );

    printf( "                 40m     41m     42m     43m     44m     45m     46m     47m\n" );

    for ( fg = 29; fg <= 37; fg++ )
    {
        for ( bold = 0; bold <= 1; bold++ )
        {
            if ( fg == 29 )
                printf( "    %sm ", bold ? "1" : " " );
            else
                printf( " %s%2dm ", bold ? "1;" : "  ", fg );

            printf( "\033[%d;%dm  mLs  \033[0m ", bold, fg );

            for ( bg = 40; bg <= 47; bg++ )
            {
                printf( "\033[%d;%d;%dm  mLs  \033[0m ", bold, bg, fg );
            }
            printf( "\n" );
        }
    }

    printf( "\n" );

    int index, red, green, blue;

    printf( "Standard colors:\n" );
    for ( index = 0; index < 16; index++ )
    {
        printf( "\033[48;5;%dm %2d ", index, index );
        if ( index == 7 )
            printf( "\033[0m\n" );
    }
    printf( "\033[0m\n" );

    printf( "\n6x6x6 Color cube:\n" );
    for ( int pass = 0; pass < 2; pass++ )
    {
        for ( green = 0; green < 6; green++ )
        {
            for ( red = pass * 3; red < pass * 3 + 3; red++ )
            {
                for ( blue = 0; blue < 6; blue++ )
                {
                    index = 16 + ( red * 36 ) + ( green * 6 ) + blue;
                    printf( "\033[48;5;%dm %2x ", index, index );
                }
                printf( "\033[0m " );
            }
            printf( "\n" );
        }
        printf( "\n" );
    }

    printf( "\nGrayscale:\n" );
    for ( index = 232; index < 256; index++ )
    {
        printf( "\033[48;5;%dm %2x ", index, index );
        if ( index == 243 )
            printf( "\033[0m\n" );
    }
    printf( "\033[0m\n\n" );

    return 0;
}
