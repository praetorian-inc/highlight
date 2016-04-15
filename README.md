## Synopsis

A utility to create an image from a text stream, automatically draw boxes around user defined content and automatically blur sensitive content.

Sometimes we have a text stream, such as the output of a configuration file, and we want to include that as an image into a document.  At the same time, we might want to highlight a particualr string of text that's found and we might want to hide other details that might contain things such as passwords.  We could use a screen capture utility and then proceed with marking up the image.  This leads to inconsistent boxes around text and certainly does not lend itself to automation.

With this utility, the entire process can be automated.

## Code Example

**cat types.h | highlight -o output.bmp -i trace**

![Simple Image](https://github.com/cryptogiff/highlight/blob/master/output.bmp)

**ps | highlight -o output_full.bmp -d 120x30 -b white -f c0ffee -x blue -i -r CMD ttys003**

![Complex Image](https://github.com/cryptogiff/highlight/blob/master/output_full.bmp)

## Motivation

I'm frequently creating reports which need images that are derived from a text file.  At the same time, some data needs to be highlighted for a customer so they can see the pertinent data.  Once in a while, there is sensitive data such as passwords which need to be blurred out.

## Installation

**make install**

It will install into /usr/local/bin and it's designed to be self contained without additional libraries.  It's been tested on Mac and a couple variants of Unix.

## Help
```
Usage: highlight [options] <string to find>
 -b color   Background color (default black)
 -c int     Keep this many lines before and after found for context
 -d WxH     Dimensions as <width>x<height, default 80x25
 -f color   Foreground color (default light green)
 -h         Help
 -g int     Greedy consuption of strings that are found:
              0 (default) exact strings
              1 extend leading and trailing alphabet and numbers
              2 extend leading and trailing until spacing
 -i         Case insensitive search
 -o file    Output image to a file (when matches found or blurring)
 -r string  Blur everything below this found string (i.e. Password)
 -v int     Verbose level (default 0), larger is more
 -x color   Box color (default red)

At least blur (-r) or a search string must be specified.
Colors may be specified as an RGB tuple, i.e. -f c0ffee

Returns number of matches in the $? shell variable.
```

## Design Descisions

Simply put, I didn't want to include libraries since they tend to be large or hard to install.  This allows the program to readily compile on different platforms.  Additionally, the BMP format has a compressed mode which does a reasonably good job.

As to using C for the language, it's one I'm very comfortable using.

The output is tailored to my needs, hence the little details with the font choice, default colors and even the thin black line around the image.
