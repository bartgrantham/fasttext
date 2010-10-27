//gcc -I/usr/include/freetype2 -I/usr/include/cairo textrender.c /usr/lib/libfreetype.so /usr/lib/libcairo.so /usr/local/lib/libfcgi.so -o textrender
/*
- FT_Face * guessfont(const char * family, const char * style) - would probably need faces_byfile, faces_byfamily, faces_bystyle
- help: "fonts", "usage", "limits", "debug" (spits out parsed params, other system metrics, processing time, only allowable via --debug)
- --extra: send back extra data (timing info, etc.) in headers
- fg/bg = HHH or HHHH or HHHHHH or HHHHHHHH
- default font?
- a better solution than qs_value/qs_decode as they are is to have:
    qs_parse: given a pointer to an array of strings, puts the location of each k/v pair in the array, then sorts by key
    qs_decode: works as before, but operates on all k/v pairs
    qs_value: finds the value based on key

NEXT:
    IS_HEX macro
    HEX2DEC macro
*/

#include "fasttext.h"

char default_font_path[] = "/usr/share/fonts/";
extern char hash_next_magic[];

FT_Library library; 
struct hash_entry * FTfaces = NULL, * Cfaces = NULL, * faces = NULL;
extern char **environ;

int main(int argc, char ** argv)
{
    char **files;
    char path[MAXPATHLEN];
    int error, count, i;

    cairo_surface_t *surface;
    cairo_text_extents_t extents;
    cairo_t *cr;

    tr_params render_params;

    error = FT_Init_FreeType( &library ); 

    // ENOTDIR, ENOENT, EACCES
    getcwd(path, MAXPATHLEN);
    if ( argc > 1 ) {  count = scanfont(argv[1], files);  }
               else {  count = scanfont(default_font_path, files);  }
    chdir(path);

    show_fonts(faces);

// FCGI loop:
    while(FCGI_Accept() >= 0)
    {
//        printf("Content-type: image/png\r\n\r\n");
        printf("Content-type: text/html\r\n\r\n");
//        printf("Content-type: text/html\r\n\r\n"
//               "Request number %d running on host <i>%s</i> - %s<br /><br />\n",
//                ++count, getenv("SERVER_NAME"), getenv("QUERY_STRING"));

//        get_params("text=bleh+bleh &font=Vera.ttf&size=40.3&fg=FFFFFF&bg=000&hash=24abcdefz&rot=123.1&rt=asdf", &render_params);
        get_params(getenv("QUERY_STRING"), &render_params);
        if ( render_params.font != NULL ) {  printf("text to render: %s<br />\n", render_params.text);  }

        for(i=0;environ[i]!=NULL;i++)
        {
//            printf("%s<br>\n", environ[i]);
        }
        draw(&render_params);
        bzero(&render_params, sizeof(render_params));
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
    face->cface = cairo_ft_font_face_create_for_ft_face(face->ftface, FT_LOAD_FORCE_AUTOHINT);

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


static cairo_status_t stdio_write(void *base, const unsigned char *data, unsigned int length)
{
    stdio_stream_t *stream = (stdio_stream_t *) base;

// why does this generate a warning?
//    if (fwrite (data, 1, length, stdout) != length)
//        return CAIRO_STATUS_WRITE_ERROR;
    return CAIRO_STATUS_SUCCESS;
}

int draw(tr_params * render)
{
    fontface * face;
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_status_t status;

    face = hash_get(faces, render->font);

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 640, 480);
    cr = cairo_create(surface);

//    cairo_set_font_face(cr, face->cface);
//    cairo_set_font_size (cr, 10.0);
//    cairo_move_to (cr, 10.0, 135.0);

    // convert to cairo_show_glyphs(), the "real" text API in cairo
//    cairo_show_text (cr, render->text);

   status = cairo_surface_write_to_png_stream(surface, stdio_write, NULL);
   printf("foo<br>\n");
//CAIRO_STATUS_SUCCESS if the PNG file was written successfully. Otherwise, CAIRO_STATUS_NO_MEMORY is returned if memory could not be allocated for the operation, 
//CAIRO_STATUS_SURFACE_TYPE_MISMATCH if the surface does not have pixel contents. 
//    cairo_surface_write_to_png_stream(surface, stdio_write, NULL);

//  free cairo surface
}

/*
// mangles string passed in on qs, ie. decodes in place
// make sure to record where all the values are beforehand
char * qs_decode(char * qs)
{
    int i=0, j=0;
    unsigned char H1, H2;
    if ( qs == NULL ) {  return NULL;  }
    while( (qs[i] != 0) && (qs[i] != '&') )
    {
        if ( qs[i] == '+' ) {  qs[j] = ' ';  }
        else if ( qs[i] == '%' )
        {
            H1 = qs[i+1];  H2 = qs[i+2];
            // in case of decode error, just return however much we've decoded so far
            if ( (H1<'0') || ('9'<H1 && H1<'A') || ('F'<H1 && H1<'a') || ('f'<H1) ) {  break;  }
            if ( (H2<'0') || ('9'<H2 && H2<'A') || ('F'<H2 && H2<'a') || ('f'<H2) ) {  break;  }

            // safer than scanf
            if ( H1 >= '0' && H1 <= '9' ) {  qs[j]  = (H1-48)*16;  }
            if ( H1 >= 'A' && H1 <= 'F' ) {  qs[j]  = (H1-55)*16;  }
            if ( H1 >= 'a' && H1 <= 'f' ) {  qs[j]  = (H1-87)*16;  }
            if ( H2 >= '0' && H2 <= '9' ) {  qs[j] += H2-48;  }
            if ( H2 >= 'A' && H2 <= 'F' ) {  qs[j] += H2-55;  }
            if ( H2 >= 'a' && H2 <= 'f' ) {  qs[j] += H2-87;  }
            i+=2;
        }
        else {  qs[j] = qs[i];  }
        i++;  j++;
    }
    qs[j] = 0;    // terminate the string, might clobber the '&' delimiter
    return qs;
}

char * qs_value(char * qs, const char * key, int * length)
{
    int i, keylen;
    char * remaining;

    if ( qs == NULL ) {  *length = 0;  return NULL;  }
    remaining = qs;
    keylen = strlen(key);
    while ( remaining != NULL )
    {
        i = strcspn(remaining, "&=");
        if ( strncmp(remaining, key, MAX(keylen,i)) == 0 )
        {
            //  if either of the next two chars after the key is &, return zero and the key+1
            i = strlen(key);
            if ( remaining[i] == '&' || remaining[i+1] == '&' ) {  *length = 0;  }
                                                           else {  *length = strcspn(remaining + i + 1, "&=");  }
            return remaining + i + 1;
        }
        if ( remaining = strchr(remaining, '&') ) {  remaining++;  }
    }
    return NULL;
}
*/
int get_params(char * qs, tr_params * render)
{
    int i;
    char * kvpairs[256];
    size_t qs_length;
    char * valptr[14]; // size, theta, w, h, x, y, r/g/b/a, bg r/g/b/a
    char * endptr;

    i = qs_parse(qs, kvpairs, 256);

    qs_length = strlen(qs);

    // First we find the locations of all the values in the k/v pairs in the qs
    // h / help
    render->help = qs_k2v("h", kvpairs, i);
    if ( render->help == NULL )  render->help = qs_k2v("help", kvpairs, i);

    // t / text
    render->text = qs_k2v("t", kvpairs, i);
    if ( render->text == NULL )  render->text = qs_k2v("text", kvpairs, i);

    // f / font
    render->font = qs_k2v("f", kvpairs, i);
    if ( render->font == NULL )  render->font = qs_k2v("font", kvpairs, i);

    // s / size
    valptr[0] = qs_k2v("s", kvpairs, i);
    if ( valptr[0] == NULL )  valptr[0] = qs_k2v("size", kvpairs, i);

    valptr[1] = qs_k2v("th", kvpairs, i);    // theta
    valptr[2] = qs_k2v("w", kvpairs, i);     // width
    valptr[3] = qs_k2v("h", kvpairs, i);     // height
    valptr[4] = qs_k2v("x", kvpairs, i);     // x offset
    valptr[5] = qs_k2v("y", kvpairs, i);     // y offset
    valptr[6] = qs_k2v("r", kvpairs, i);     // text color red
    valptr[7] = qs_k2v("g", kvpairs, i);     // text color green
    valptr[8] = qs_k2v("b", kvpairs, i);     // text color blue
    valptr[9] = qs_k2v("a", kvpairs, i);     // text color alpha
    valptr[10] = qs_k2v("bgr", kvpairs, i);  // background color red
    valptr[11] = qs_k2v("bgg", kvpairs, i);  // background color green
    valptr[12] = qs_k2v("bgb", kvpairs, i);  // background color blue
    valptr[13] = qs_k2v("bga", kvpairs, i);  // background color alpha


    // Second, we decode and convert where necessary.  qs_decode mangles the
    // string it is passed, so all calls to it come after we've recorded
    // the locations of the values, as above

    qs_decode(render->help);
    qs_decode(render->text);
    qs_decode(render->font);
    render->size = qs2d(valptr[0], _DEFAULT_FONT_SIZE);

    render->th = qs2d(valptr[1], _DEFAULT_THETA);
    render->w = qs2d(valptr[2], _DEFAULT_WIDTH);
    render->h = qs2d(valptr[3], _DEFAULT_HEIGHT);
    render->x = qs2d(valptr[4], _DEFAULT_X);
    render->y = qs2d(valptr[5], _DEFAULT_Y);

    render->r = CLAMP(qs2l(valptr[6], _DEFAULT_R), 0, 255);
    render->g = CLAMP(qs2l(valptr[7], _DEFAULT_G), 0, 255);
    render->b = CLAMP(qs2l(valptr[8], _DEFAULT_B), 0, 255);
    render->a = CLAMP(qs2d(valptr[9], _DEFAULT_A), 0.0, 1.0);

    render->bgr = CLAMP(qs2l(valptr[10], _DEFAULT_BGR), 0, 255);
    render->bgg = CLAMP(qs2l(valptr[11], _DEFAULT_BGG), 0, 255);
    render->bgb = CLAMP(qs2l(valptr[12], _DEFAULT_BGB), 0, 255);
    render->bga = CLAMP(qs2d(valptr[12], _DEFAULT_BGA), 0.0, 1.0);
}

// strtod/strtol don't gracefully handle null pointers, but we also can't know
// if they succeed unless we check the value of endptr, so to minimize boilerplate
// for now we use these strange inline contraptions
// NOTE: calls qs_decode, which is destructive to the string it is passed
inline double qs2d(char * qs, double def)
{
    double d;
    char * endptr;
    if ( qs == NULL )  {  return def;  }
    d = strtod(qs, &endptr);
    return (endptr == qs) ? def : d;
}

// NOTE: calls qs_decode, which is destructive to the string it is passed
inline long qs2l(char * qs, long def)
{
    long l;
    char * endptr;
    if ( qs == NULL )  {  return def;  }
    l = strtol(qs, &endptr, 0);
    return (endptr == qs) ? def : l;
}
