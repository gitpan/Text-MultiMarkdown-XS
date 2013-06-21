#!/usr/bin/env perl

use blib;
use Test::More;
use Text::MultiMarkdown::XS;

my $input = <<EOS;
Heading
-------

text
EOS

my $latex_output = <<EOS;
\\chapter{Heading}
\\label{heading}

text
EOS

my $beamer_output = <<EOS;
\\section{Heading}
\\label{heading}

text
EOS

my $memoir_output = <<EOS;
\\chapter{Heading}
\\label{heading}

text
EOS

chomp($latex_output);
chomp($beamer_output);
chomp($memoir_output);

is(markdown($input, { output => 'latex'  } ), $latex_output,  'latex');
is(markdown($input, { output => 'beamer' } ), $beamer_output, 'beamer');
is(markdown($input, { output => 'memoir' } ), $memoir_output, 'memoir');


done_testing();
