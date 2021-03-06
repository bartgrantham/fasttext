fasttext
========
fasttext is a FastCGI server that serves images of dynamically rendered text


Description
-----------
fasttext is a FastCGI server, written in C, that takes parameters on the
HTTP query string and renders text into a png that it serves back through
the webserver.  It uses Cairo2 with Freetype, supports TrueType font files,
and is about as fast as reasonably possible, the bottleneck in rendering
being the png compression.  It is intended to be used behind a caching web
server or proxy, but can be used directly for low-traffic sites.


How To Use
----------

Quick Examples:

http://fasttext.example.com/?t=this+is+a+test&s=30&g=.1&b=.4&a=.5
-> produces "this is a test" in tuffy, 30 pixels tall in a light purple on
a transparent background

http://fasttext.example.com/?t=hello,+world&s=50&h=30&w=150&bg=888
-> produces "hello, world" in tuffy, 50 pixels tall in black on a half-transparent
gray background, but clipped halfway up the lettering and down the
middle of the "w" in "world"

http://fasttext.example.com/?f=Vera.ttf&t=it+is+not+the+critic+who+counts&s=40
-> produces "it is not the critic who counts" in Vera Regular, 100 pixels
tall in opaque black on a transparent background (provided the server has
Vera.ttf somewhere in the default fonts directory, /usr/share/fonts/)


The server will accept the following parameters:

    't' or 'text'      : Text to render
                         Defaults to '', maximum of 511 characters

    'f' or 'font'      : Typeface to use (see below for font file name mapping),
                         defaults to Thatcher Ulrich's tuffy font: http://tulrich.com/fonts/
                         The default is also mapped to the font name "__DEFAULT__"

    's' or 'size'      : Size of the rendered text in pixels
                         Defaults to 12.0, maximum is 1440.0

    'w', 'h'           : Width and height of the image
                         Defaults to minimum extents of the text, maximum is
                         2000x2000

    'x', 'y'           : x and y offset of the text within the image, in pixels
                         from the upper-left corner
                         Defaults to 0,0, no real limits other than drawing
                         outside the maximum image size will result in a large
                         blank image

    'fg'               : 3/4 or 6/8 letter hex color for the text drawing color
                         in RGB(A) format.
                         Defaults to opaque black
                         Completely overrides individual color parameters

    'r', 'g', 'b', 'a' : Individual color values for red, green, blue, and alpha,
                         in decimal 0.0 to 1.0 (clamped if outside this range)
                         Defaults to 0.0 for r/g/b, 1.0 for alpha (opaque black)

    'bg'               : 3/4 or 6/8 letter hex color for the background color
                         in RGB(A) format.
                         Defaults to transparent black
                         Completely overrides individual bg color parameters

    'bgr', 'bgg'       : Individual background color values for red, green, blue,
    'bgb', 'bga'         and alpha in decimal 0.0 to 1.0 (clamped if outside range)
                         Defaults to 0.0 for each component


On server startup fasttext will scan for fonts in either the built-in
default_font_path[] or whatever path is passed as the first parameter.
It will use the fonts' file names, minus the directory, as the name by
which the user should refer to the font (ie. "/usr/share/fonts/Vera.ttf"
is utilized with "f=Vera.ttf").  For this reason I recommend creating
symlinks for the font files you wish to make available:

    # ln -s /usr/share/fonts/Vera.ttf /path/to/fasttext/default/font/dir/v

This allows for a degree of obfuscation, but more importantly cleans
up the query string:

<http://fasttext.example.com/?f=VeraMoBd.ttf&t=this+is+a+test>
vs.
<http://fasttext.example.com/?f=vb&t=this+is+a+test>



Installation
------------

First you'll need to have cairo2 with freetype installed.  Under Ubuntu 10.10
Server libcairo2-dev includes libfreetype6-dev, so all you need is:

    # sudo apt-get -y install libcairo2-dev

Next, you'll need FastCGI installed.  Again under Ubuntu 10.10 server:

    # sudo apt-get -y install libfcgi-dev

And now you can build the fasttext binary with just:

    # make

If you want, you can copy the resulting binary, "fasttext" whereever you
prefer.  I suggest a directory dedicated to fastcgi binaries at the same
level as the webserver root.  Don't forget to copy whatever font you'd like
as the default into DEFAULT.ttf into the same directory.  Otherwise you'll
get 500 if you don't specify the font face in the query string.  I've
included Thatcher Ulrich's public domain "Tuffy" as a basic default, you will
want to change this to whatever your site's default branding font is.  The
default fonts directory is /usr/share/fonts, but I suggest using an alternative
directory that has symlinks to the fonts you want to make available for use.

Now you need to setup your webserver.  I like lighttpd, but any webserver that
supports FastCGI should work.  Here's a lighttpd config example:

    $HTTP["host"] == "fasttext.yourdomain.com" {
      $HTTP["referer"] !~ ".*yourdomain.com" {
        url.rewrite-once = ( ".*" => "/?t=I will only render for yourdomain.com referers" )
      }
      url.rewrite-once = ("(\?.*)" => "/$1")
      fastcgi.server = (
                         "" =>
                           ( "" =>
                             (
                               "socket" => "/var/run/fastcgi/fasttext.socket",
                               "bin-path" => "/www/fastcgi/fasttext/fasttext",
                               "max-procs" => 4
                             )
                           )
                         )
    }


Now, give it a try at <http://fasttext.yourdomain.com/?t=Hello,+World!>


Bugs, etc.
----------
- The code for rotation is commented-out because it rotates around the upper-
    left corner, instead of text-center as expected.  Should be an easy fix.

- fasttext doesn't currently handle newlines gracefully.  This is because I'm
    using Cairo's simpler text API instead of Pango.

- Since it doesn't support newlines properly, there's a lot of code intended
    to draw a help and a debugging placard as an image that isn't currently
    enabled.  Once newlines are handled properly this will be a nice new
    feature.

- I'm sure there are more that I'm forgetting...

Please let me know if you find any, or if you have license-friendly
enhancements to add.


License
-------
MIT License.  See ./LICENSE or
<http://www.opensource.org/licenses/mit-license.php>

Few things are more enjoyable than the knowledge that you've helped another
person. If you do use these functions for anything, I'd love to hear about it:

<bart@bartgrantham.com>


Enjoy!

-Bart
