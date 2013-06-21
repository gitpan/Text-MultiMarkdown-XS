#!/usr/bin/env perl

use strict;
use blib;
use Test::More;
use Text::MultiMarkdown::XS;

is($mmd_version, '4.1.1', '$mmd_version');

is(EXT_COMPATIBILITY,  1 << 0, "EXT_COMPATIBILITY");
is(EXT_SMART,          1 << 3, "EXT_SMART");
is(EXT_NOTES,          1 << 4, "EXT_NOTES");
is(EXT_NO_LABELS,      1 << 5, "EXT_NO_LABELS");
is(EXT_FILTER_HTML,    1 << 7, "EXT_FILTER_HTML");
is(EXT_FILTER_STYLES,  1 << 6, "EXT_FILTER_STYLES");
is(EXT_PROCESS_HTML,   1 << 8, "EXT_PROCESS_HTML");

my $enum = 0;
is(HTML_FORMAT,        $enum++, "HTML_FORMAT");
$enum++; #is(TEXT_FORMAT,        $enum++, "TEXT_FORMAT");
is(LATEX_FORMAT,       $enum++, "LATEX_FORMAT");
is(MEMOIR_FORMAT,      $enum++, "MEMOIR_FORMAT");
is(BEAMER_FORMAT,      $enum++, "BEAMER_FORMAT");
is(OPML_FORMAT,        $enum++, "OPML_FORMAT");
is(ODF_FORMAT,         $enum++, "ODF_FORMAT");
is(RTF_FORMAT,         $enum++, "RTF_FORMAT");
is(MAN_FORMAT,         $enum++, "MAN_FORMAT");
is(ORIGINAL_FORMAT,    $enum++, "ORIGINAL_FORMAT");



done_testing();
