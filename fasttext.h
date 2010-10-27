#include "fcgi_stdio.h"
//#include <stdio.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <cairo.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "qs/qs.h"
#include "hash/hash.h"


#ifndef __TEXTRENDER_H_
#define __TEXTRENDER_H_

#define MAX_TEXT_LEN 255
#define MAX_FONT_LEN 255
#define _DEFAULT_FONT_SIZE 12.0
#define _DEFAULT_THETA 1.0
#define _DEFAULT_WIDTH 2.0
#define _DEFAULT_HEIGHT 3.0
#define _DEFAULT_X 4.0
#define _DEFAULT_Y 5.0

// default text == black
#define _DEFAULT_R 3
#define _DEFAULT_G 4
#define _DEFAULT_B 6
#define _DEFAULT_A 0.8

// default text == white
#define _DEFAULT_BGR 255
#define _DEFAULT_BGG 255
#define _DEFAULT_BGB 255
#define _DEFAULT_BGA 0.4


// lots of systems define ABS/MIN/MAX/CLAMP, so they get wrapped in ifdefs
#ifndef ABS
#define ABS(a)  (((a) < 0) ? -(a) : (a))
#endif

#ifndef MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif

#ifndef CLAMP
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#endif


typedef struct
{
    char * text;
    char * font;
    char * help;
    double  size;
    double  th;    // theta, angle of rotation
    double  w, h;
    double  x, y;
    int  r, g, b;
    double a;
    int  bgr, bgg, bgb;
    double bga;
} tr_params;


typedef struct
{
    FT_Face * ftface;
    cairo_font_face_t * cface;
} fontface;


char usage[] = "Usage:\n\nYou must use parameters";

/*
char * qs_value(char * qs, const char * key, int * length);

char * qs_decode(char * qs);
*/
inline double qs2d(char * qs, double def);

inline long qs2l(char * qs, long def);

int relative_dir(struct direct *e);

int scanfont(char * font_dir, char ** files);

int addfont(char * filepath);

typedef struct stdio_stream stdio_stream_t;

int draw();

static cairo_status_t stdio_write(void *base, const unsigned char *data, unsigned int length);

void show_fonts(struct hash_entry h[]);


#endif  // __TEXTRENDER_H_
