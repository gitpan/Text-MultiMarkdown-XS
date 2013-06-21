#!/usr/bin/env perl

# Test processing of smart quotes
# todo: check locale-specific formatting (German, French, etc)

use blib;
use Test::More;
use Text::MultiMarkdown::XS;

my $input = <<EOS;
Heading
-------

Here is some "quoted text".
EOS

my $output_quotes_enabled = <<EOS;
<h2 id="heading">Heading</h2>

<p>Here is some &#8220;quoted text&#8221;.</p>
EOS

my $output_quotes_disabled = <<EOS;
<h2 id="heading">Heading</h2>

<p>Here is some &quot;quoted text&quot;.</p>
EOS

chomp($output_quotes_enabled);
chomp($output_quotes_disabled);

is(markdown($input, { smart => 1  } ), $output_quotes_enabled,  'smart quotes enabled');
is(markdown($input, { smart => 0  } ), $output_quotes_disabled, 'smart quotes disabled');
is(markdown($input, {             } ), $output_quotes_enabled,  'smart quotes default setting');

done_testing();
