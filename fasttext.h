#include "fcgi_stdio.h"
//#include <stdio.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <cairo.h>
#include <cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "qs/qs.h"
#include "hash/hash.h"


#ifndef __TEXTRENDER_H_
#define __TEXTRENDER_H_

#define MAX_TEXT_LEN 255
#define MAX_FONT_LEN 255
#define _DEFAULT_FONT_SIZE 12.0
#define _DEFAULT_THETA 0.0
#define _DEFAULT_WIDTH 0
#define _DEFAULT_HEIGHT 0
#define _DEFAULT_X 0.0
#define _DEFAULT_Y 0.0

// default text == opaque black
#define _DEFAULT_R 0
#define _DEFAULT_G 0
#define _DEFAULT_B 0
#define _DEFAULT_A 1.0

// default text == transparent white
#define _DEFAULT_BGR 255
#define _DEFAULT_BGG 255
#define _DEFAULT_BGB 255
#define _DEFAULT_BGA 0.0


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
    char * fmt;
    double  size;
    double  th;    // theta, angle of rotation
    int  w, h;
    double  x, y;
    int  r, g, b;
    int  bgr, bgg, bgb;
    double a, bga;
} tr_params;


typedef struct
{
    FT_Face * ftface;
    cairo_font_face_t * cface;
} fontface;


char usage[] = "Usage:\n\nYou must use parameters";

int draw(tr_params * render);

static cairo_status_t FCGI_cairo_write_stream (void * in_closure, const unsigned char *data, unsigned int length);

inline double qs2d(char * qs, double def);

inline long qs2l(char * qs, long def);

int relative_dir(struct direct *e);

int scanfont(char * font_dir, char ** files);

int addfont(char * filepath);

void show_fonts(struct hash_entry h[]);

int get_params(char * qs, tr_params * render);

void dump_params(tr_params * render);


#endif  // __TEXTRENDER_H_
