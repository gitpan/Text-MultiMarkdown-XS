package Text::MultiMarkdown::XS;

use strict;
use Carp;
use vars qw(@constants);

require Exporter;
require DynaLoader;

our $VERSION = "0.001_03";
our @ISA     = qw(Exporter DynaLoader);

BEGIN {
    our @constants  = qw( EXT_COMPATIBILITY
                          EXT_COMPLETE
                          EXT_FILTER_HTML
                          EXT_FILTER_STYLES
                          EXT_NOTES     
                          EXT_NO_LABELS
                          EXT_NO_METADATA
                          EXT_OBFUSCATE
                          EXT_PROCESS_HTML
                          EXT_SMART

                          HTML_FORMAT
                          TEXT_FORMAT
                          LATEX_FORMAT
                          MEMOIR_FORMAT
                          BEAMER_FORMAT
                          MAN_FORMAT
                          ODF_FORMAT
                          OPML_FORMAT
                          RTF_FORMAT
                          ORIGINAL_FORMAT
                         );
}
use subs @constants;

our @EXPORT  = ( 'markdown', '$mmd_version', @constants );

__PACKAGE__->bootstrap($VERSION);

my $format_re = qr{ ^ (?: HTML | TEXT | LATEX | MEMOIR | BEAMER | MAN | ODF | OPML | RTF ) }x;
my $false_re  = qr{^(?:|0|false|off)$}i;

my %option_defs = ( smart        => [ bool    => EXT_SMART ],
		    complete     => [ bool    => EXT_COMPLETE ],
		    obfuscate    => [ bool    => EXT_OBFUSCATE ],
		    use_metadata => [ invbool => EXT_NO_METADATA ],
    );


sub new {
    my $class = shift;
    my $options;
    if (@_ == 1 and ref $_[0] eq 'HASH') {
	$options = shift;
    }
    else {
	$options = { @_ };
    }
    return bless $options, ref($class) || $class;
}


sub markdown {
    my $self = shift;

    # Detect functional mode, and create an instance for this run..
    unless (ref $self) {
        if ( $self ne __PACKAGE__ ) {
            my $ob = __PACKAGE__->new();
                                # $self is text, $text is options
            return $ob->markdown($self, @_);
        }
        else {
            croak('Calling ' . $self . '->markdown (as a class method) is not supported.');
        }
    }

    my ($text, $options) = @_;

    my $extensions    = EXT_SMART;
    my $output_format = HTML_FORMAT;

    $options ||= {};

    if (my $value = uc($options->{output} || '')) {
        if ($value =~ $format_re) {
            $output_format = constant("${value}_FORMAT");
        }
    }

    foreach my $option (keys %option_defs) {
	if (exists $options->{$option}) {
	    my $value = $options->{$option} || '';
	    my ($type, $flag) = @{$option_defs{$option}};
	    if ($type eq 'bool') {
		if ($value =~ $false_re) {
		    $extensions &= ~$flag;
		}
		else {
		    $extensions |= $flag;
		}
	    }
	    elsif ($type eq 'invbool') {
		if ($value =~ $false_re) {
		    $extensions |= $flag;
		}
		else {
		    $extensions &= ~$flag;
		}
	    }
	}
    }

    return _markdown($text, $extensions, $output_format);
}


sub DESTROY {}

sub AUTOLOAD {
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.

    my $constname;
    our $AUTOLOAD;
    ($constname = $AUTOLOAD) =~ s/.*:://;
    croak "&Text::MultiMarkdown::XS::constant not defined" if $constname eq 'constant';
    my ($error, $val) = constant($constname);
    if ($error) { croak $error; }
    {
        no strict 'refs';
        *$AUTOLOAD = sub { $val };
    }
    goto &$AUTOLOAD;
}


1;

=head1 NAME

Text::MultiMarkdown::XS

=head1 SYNOPSIS

  use Text::MultiMarkdown::XS 'markdown';

  my $html  = markdown( $text, { output => 'html' } );

  my $latex = markdown( $text, { output => 'latex' } );


=head1 DESCRIPTION

B<Warning>: this is the first, alpha release of the module and the interface is liable to
change without notice.

C<Text::MultiMarkdown::XS> is a wrapper around Fletcher T. Penney's MultiMarkdown library
(version 4).

=head1 OPTIONS

The C<markdown()> function takes a number of options:

=over 4

=item C<output>

the output format - a case-insensitive string, which may take one of the following values:

=over 4

=item C<HTML>

=item C<LaTeX>

=back

=item C<complate>

boolean value to specify whether a complete document or a fragment should be generated


=item C<smart>

boolean value to specify whether I<smart> quotes should be enabled.

=back

The following values are accepted as boolean false values: C<undef>, 0, 'false' or 'off'
(string values are case-insensitive). Anything else is regarded as true.


=head1 AUTHOR

Andrew Ford <andrewf@cpan.org>

MultiMarkdown library by Fletcher Penney
<https://github.com/fletcherr/MultiMarkdown-4/>

The original Markdown implementation was by John Gruber
<http://daringfireball.net/>



=head1 COPYRIGHT AND LICENSE

Text::MultiMarkdown::XS Copyright (C) 2013 Andrew Ford
All rights reserved.

MultiMarkdown library Copyright (C) 2010-2013 Fletcher T. Penney with portions copyright
by Daniel Jalkut and John MacFarlane.  See the file MultiMarkdown-License for details.

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

See L<http://www.perl.com/perl/misc/Artistic.html>

