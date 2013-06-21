#!/usr/bin/env perl

use blib;
use Test::More;
use Text::MultiMarkdown::XS;

my $input = <<EOS;
heading
-------

text
EOS

my $output = <<EOS;
<h2 id="heading">heading</h2>

<p>text</p>
EOS

chomp($output);

is(markdown($input), $output);



done_testing();
