Text-MultiMarkdown-XS
=====================

This is an initial version (0.001_01) of a Perl module that wraps Fletcher T. Penney's
MultiMarkdown 4 library.

Basic usage is:

    use Text::MultiMarkdown::XS qw(markdown);

    my $html = markdown($text, $options);

where $options is a hash reference.  Valid keys are:

* `output`: the output format (case insensitive string) - one of `html` (default), `text`,
   `latex`, `beamer`, `memoir`
* `complete`: boolean indicating whether to output a complete document or just a fragment
  (default is to output a fragment)
* `obfuscate`: boolean indicating whether `mailto:` links should be obfuscated (default is false)
* `smart`: boolean indicating whether smart quote processing should be enabled (default is false)
* `use_metadata`: boolean to control whether metadata at the start of the input text is
  processed (default is true)

A false value for a boolean option can be specified as `undef`, `0`, `"false"`, or
`"off"`.  Any other value is taken to be true.  The string values `"false"` and `"off"`
are case-insensitive.

More options will be added in future versions of the module.

The module includes the beginnings of a _man_ output option.  This compiles but is
non-functional.  When it is complete it will be submitted to the MultiMarkdown-4 repo.


Build instructions
------------------

Standard Perl build sequence:

    perl Makefile.PL
    make
    make test
    make install

Building requires a C compiler.


To Do
-----

* more complete set of options that map to the library's options
* include Fletcher's C code as a git sub repository (need to get the build working with that)
* expand test suite
* integrate into Text::MultiMarkdown, so that that module loads this XS module if it is
  installed or defaults to the pure perl version.  May need to carp if the options
  supplied are not compatible with the PP version.