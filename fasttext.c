/*
- should I build a: FT_Face * guessfont(const char * family, const char * style) - would probably need faces_byfile, faces_byfamily, faces_bystyle
- wrap debug text so that it only displays on --debug execution
- --extra: send back extra data (timing info, etc.) in headers
- fg/bg = HHH or HHHH or HHHHHH or HHHHHHHH
- default font?
- does cairo support gif?  If so I guess bg color is a blend color and the true background is transparent if alpha > 0
- why can't I output to error until accept is called?
- how to free/destroy cairo_surface?
- how to set fill color?
- how to text rotation?
- most of the max params aren't hooked up
- convert to cairo_show_glyphs(), aka PANGO, the "real" text API in cairo
*/

#include "fasttext.h"

char default_font_path[] = "/usr/share/fonts/";
extern char hash_next_magic[];

FT_Library library; 
struct hash_entry * faces = NULL;
extern char **environ;
//cairo_surface_t *surface;
//cairo_t *cr;

int main(int argc, char ** argv)
{
    char **files;
    char path[MAXPATHLEN];
    int error, count, i;

    tr_params render_params;
//    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 300, 300);
//    cr = cairo_create(surface);

    error = FT_Init_FreeType( &library ); 

    // ENOTDIR, ENOENT, EACCES
    getcwd(path, MAXPATHLEN);
    if ( argc > 1 ) {  count = scanfont(argv[1], files);  }
               else {  count = scanfont(default_font_path, files);  }
    chdir(path);

    fprintf(stderr, "testo: %d", 1234);

//    show_fonts(faces);

    // FCGI loop:
    while(FCGI_Accept() >= 0)
    {
        bzero(&render_params, sizeof(render_params));

//        fprintf(stderr, "log: %d", 2345);

        get_params(getenv("QUERY_STRING"), &render_params);

        if ( render_params.help != NULL && (strncmp(render_params.help, "textdebug", 10) == 0) )
        {
            printf("Content-type: text/html\n\n%s", help_text(&render_params));
        }
        else
        {
            printf("Content-type: image/png\n\n");
            draw(&render_params);
        }

//        for(i=0;environ[i]!=NULL;i++)  printf("%s<br>\n", environ[i]);
    }
}


int scanfont(char * font_dir, char ** files)
{
    int count = 0;
    struct dirent * entry;
    char path[MAXPATHLEN];
    DIR * dir;

    dir = opendir(font_dir);
    chdir(font_dir);
    if ( dir == NULL ) {  perror("opendir");  printf(" %s\n", font_dir);  }
    while(entry = readdir(dir))
    {
        if ( entry->d_type == DT_DIR && ! relative_dir(entry) )
        {
            getcwd(path, MAXPATHLEN);
            count += scanfont(entry->d_name, files);
            chdir(path);
        }
        if ( entry->d_type == DT_REG || entry->d_type == DT_LNK )
        {
            getcwd(path, MAXPATHLEN);
            strcat(path, "/");  strcat(path, entry->d_name);  // there's got to be a better way than this...
            addfont(path);
            count++;
        }
    }
    return count;
}


int addfont(char * filepath)
{
    fontface * face;
    int error;

    face = malloc(sizeof(fontface));
    if ( face == NULL ) {  return ENOMEM;  }

    face->ftface = malloc(sizeof(FT_Face));
    if ( face->ftface == NULL ) {  return ENOMEM;  }

    error = FT_New_Face(library, filepath, 0, face->ftface);
    if ( error )
    {
        free(face->ftface);
        free(face);
        printf("cannot load font %s : Error code %d\n", filepath, error);
        return error;
    }

    // why does this generate a warning?
    face->cface = cairo_ft_font_face_create_for_ft_face(*(face->ftface), FT_LOAD_FORCE_AUTOHINT);

//    printf("Added \t%s\t%s\t%s\n", strrchr(filepath, '/')+1, (*(face->ftface))->family_name, (*(face->ftface))->style_name);

    hash_set(faces, strrchr(filepath, '/')+1, face);

    return 0;
}


void show_fonts(struct hash_entry h[])
{
    int i;
    if ( h == NULL ) {  return;  }
    for(i=0; i<HASH_KEYS_PER_TABLE; i++)
    {
        if ( h[i].key == NULL ) {  continue;  }
        if ( h[i].key == hash_next_magic ) {  show_fonts(h[i].value);  }
        else {  printf("%s\t%p\t%s\n", h[i].key, h[i].value, (*(((fontface*)h[i].value)->ftface))->family_name);  }
    }
}


int relative_dir(struct direct *e)
{  return ((strcoll(e->d_name, ".") == 0) || (strcoll(e->d_name, "..") == 0))?1:0;  }


static cairo_status_t FCGI_cairo_write_stream (void * in_closure, const unsigned char *data, unsigned int length)
{
    if ( length == fwrite((void *)data, sizeof(char), length, stdout))
        return CAIRO_STATUS_SUCCESS;
    else
        return CAIRO_STATUS_WRITE_ERROR;
}


int draw(tr_params * render)
{
    fontface * face;
    cairo_surface_t * surface;
    cairo_t * cr;
    cairo_status_t status;
    cairo_text_extents_t text_extents, help_extents;

    // the help parameter will probably alter the text to draw, so let's check it first
    if ( render->help != NULL && strncmp(render->help, "debug", sizeof("debug")) != 0 )
        render->text = help_text(render);

    face = hash_get(faces, render->font);

    if ( render->w == 0 ) {  render->w = 800; } //MIN(text_extents.width, MAX_WIDTH);  }
    if ( render->h == 0 ) {  render->h = 800; } //MIN(text_extents.height, MAX_HEIGHT);  }

//    cairo_clip_extents(cr, 0, 0, 400, 400);

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, render->w, render->h);
    cr = cairo_create(surface);

    // XXX if ( bgr != 255 || bgg != 255 || bgb != 255 || bga != 0.0 )
    //    fill background with params
    // else
    //    fill background with 255/255/255/0.0

    // XXX - set text bg blend color?

    // XXX - if face->cface == NULL use default

    cairo_set_font_size(cr, render->size);

    cairo_set_font_face(cr, face->cface);

    // XXX set text color
    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);

    // XXX if ( render->x != 0.0 && render->y != 0.0 ) move to x,y, otherwise move y to text height
    cairo_move_to(cr, render->x, render->y);

    // XXX if ( render->th != 0.0 )  set text rotation cairo_set_font_matrix 

    cairo_show_text(cr, render->text);

    cairo_text_extents(cr, render->text, &text_extents);
    fprintf(stderr, "extents: %f, %f", text_extents.height, text_extents.width);

    // if we're set to debug, we'll have attempted a render above, and now we draw a parameters placard
    if ( render->help != NULL && (strncmp(render->help, "debug", sizeof("debug")) == 0) )
    {
        // set font to whatever ends up being the default
        // half-transparent black fill
        cairo_set_font_size(cr, 12);
        cairo_move_to(cr, 0, 12);
        // text bg color = black
        // text color = white
        cairo_show_text(cr, help_text(render));
    }

    status = cairo_surface_write_to_png_stream(surface, FCGI_cairo_write_stream, NULL);

//  free cairo surface
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}


int get_params(char * qs, tr_params * render)
{
    static char * kvpairs[256];
    int i;
    char * valptr;

    // First we find the locations of all the values in the k/v pairs in the qs
    i = qs_parse(qs, kvpairs, 256);

    // Second we set up our parameters struct, decoding ints/floats where necessary
    // help
    render->help = qs_k2v("help", kvpairs, i);

    // t / text
    render->text = qs_k2v("t", kvpairs, i);
    if ( render->text == NULL )  render->text = qs_k2v("text", kvpairs, i);

    // f / font
    render->font = qs_k2v("f", kvpairs, i);
    if ( render->font == NULL )  render->font = qs_k2v("font", kvpairs, i);

    // fmt
    render->fmt = qs_k2v("fmt", kvpairs, i);

    // s / size
    valptr = qs_k2v("s", kvpairs, i);
    if ( valptr == NULL )  valptr = qs_k2v("size", kvpairs, i);
    render->size = qs2d(valptr, _DEFAULT_FONT_SIZE);

    // th / rot
    valptr = qs_k2v("th", kvpairs, i);
    if ( valptr == NULL )  valptr = qs_k2v("th", kvpairs, i);
    render->th = qs2d(valptr, _DEFAULT_THETA);

    // w, h, x, y
    render->w = qs2l(qs_k2v("w", kvpairs, i), _DEFAULT_WIDTH);
    render->h = qs2l(qs_k2v("h", kvpairs, i), _DEFAULT_HEIGHT);
    render->x = qs2d(qs_k2v("x", kvpairs, i), _DEFAULT_X);
    render->y = qs2d(qs_k2v("y", kvpairs, i), _DEFAULT_Y);

    // r, g, b, a
    render->r = CLAMP(qs2d(qs_k2v("r", kvpairs, i), _DEFAULT_R), 0, 255);
    render->g = CLAMP(qs2d(qs_k2v("g", kvpairs, i), _DEFAULT_G), 0, 255);
    render->b = CLAMP(qs2d(qs_k2v("b", kvpairs, i), _DEFAULT_B), 0, 255);
    render->a = CLAMP(qs2d(qs_k2v("a", kvpairs, i), _DEFAULT_A), 0.0, 1.0);

    // background r, g, b, a
    render->bgr = CLAMP(qs2d(qs_k2v("bgr", kvpairs, i), _DEFAULT_BGR), 0, 255);
    render->bgg = CLAMP(qs2d(qs_k2v("bgg", kvpairs, i), _DEFAULT_BGG), 0, 255);
    render->bgb = CLAMP(qs2d(qs_k2v("bgb", kvpairs, i), _DEFAULT_BGB), 0, 255);
    render->bga = CLAMP(qs2d(qs_k2v("bga", kvpairs, i), _DEFAULT_BGA), 0.0, 1.0);

    return 0;
}


char * help_text(tr_params * render)
{
    static char help[MAX_HELP_LEN+1];
    int help_len=0;

    // debug, textdebug
    if ( strncmp(render->help, "debug", sizeof("debug")) == 0 ||
         strncmp(render->help, "textdebug", sizeof("textdebug")) == 0 )
    {
        if ( strncmp(render->help, "textdebug", sizeof("textdebug")) == 0 )
            help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "<pre>\n");

        // include: processing time if help == "debug" (rendered), current heap usage, number of calls

        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "help : %s\n", (render->help!=NULL)?render->help:"");
        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "text : %s\n", (render->text!=NULL)?render->text:"");
        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "font : %s (%p)\n", (render->font!=NULL)?render->font:"", render->font);
        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "fmt : %s\n", (render->fmt!=NULL)?render->fmt:"");

        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "size : %f\n", render->size);

        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "w : %d\n", render->w);
        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "h : %d\n", render->h);
        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "x : %.2f\n", render->x);
        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "y : %.2f\n", render->y);
        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "theta : %.2f\n", render->th);

        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "r : %d\n", render->r);
        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "g : %d\n", render->g);
        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "b : %d\n", render->b);
        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "a : %.2f\n", render->a);

        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "bgr : %d\n", render->bgr);
        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "bgg : %d\n", render->bgg);
        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "bgb : %d\n", render->bgb);
        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "bga : %.2f\n", render->bga);

        if ( strncmp(render->help, "textdebug", sizeof("textdebug")) == 0 )
            help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "</pre>\n");

        return help;
    }

    // limits
    else if ( strncmp(render->help, "limits", sizeof("limits")) == 0 )
    {
        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "Max text: %d bytes, ", MAX_TEXT_LEN);
        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "Max font size: %f pts, ", MAX_FONT_SIZE);
        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "Max width: %d pixels, ", MAX_WIDTH);
        help_len += snprintf(help+help_len, MAX_HELP_LEN-help_len, "Max height: %d pixels ", MAX_HEIGHT);
        return help;
    }

    // fonts
    else if ( strncmp(render->help, "fonts", sizeof("fonts")) == 0 )
    {  return font_example;  }

    return usage;
}


// strtod/strtol don't gracefully handle null pointers, but we also can't know
// if they succeed unless we check the value of endptr, so to minimize boilerplate
// for now we use these strange inline contraptions
inline double qs2d(char * qs, double def)
{
    double d = 0.0;
    char * endptr;
    if ( qs == NULL )  {  return def;  }
    d = strtod(qs, &endptr);
    return (endptr == qs) ? def : d;
}

inline long qs2l(char * qs, long def)
{
    long l = 0;
    char * endptr;
    if ( qs == NULL )  {  return def;  }
    l = strtol(qs, &endptr, 0);
    return (endptr == qs) ? def : l;
}


