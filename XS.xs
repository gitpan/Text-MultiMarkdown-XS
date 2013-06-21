#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "libMultiMarkdown.h"

#include "const-defs.inc"
#include "const-c.inc"

MODULE = Text::MultiMarkdown::XS          PACKAGE = Text::MultiMarkdown::XS

PROTOTYPES: DISABLE

BOOT:
    /* mmd_version returns a malloc'ed string */
    char *version = mmd_version();
    sv_setpv(get_sv("Text::MultiMarkdown::XS::mmd_version", TRUE), version);
    free(version);

SV *
_markdown(text, extensions=0, output_format=0)
    char *text;
    int  extensions;
    int  output_format;

 INIT:
    char *result;

 CODE:
    /* markdown_to_string returns a malloc'ed string */
    result = markdown_to_string(text, extensions, output_format);
    RETVAL = newSVpv(result, 0);
    free(result);

 OUTPUT:
    RETVAL

INCLUDE: const-xs.inc




