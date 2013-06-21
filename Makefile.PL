use 5.006;
use strict;
use ExtUtils::MakeMaker;
use ExtUtils::Constant qw (WriteConstants autoload);

my $module_name = 'Text::MultiMarkdown::XS';

my @constants = ( qw( EXT_COMPATIBILITY
                      EXT_COMPLETE      
                      EXT_HEAD_CLOSED   
                      EXT_SMART         
                      EXT_NOTES         
                      EXT_NO_LABELS     
                      EXT_FILTER_STYLES 
                      EXT_FILTER_HTML   
                      EXT_PROCESS_HTML  
                      EXT_NO_METADATA   
                      EXT_OBFUSCATE     
                      EXT_CRITIC        
                      EXT_CRITIC_ACCEPT 
                      EXT_CRITIC_REJECT 
                      EXT_FAKE          

                      HTML_FORMAT
                      TEXT_FORMAT
                      LATEX_FORMAT
                      MEMOIR_FORMAT
                      BEAMER_FORMAT
                      MAN_FORMAT
                      OPML_FORMAT
                      ODF_FORMAT
                      RTF_FORMAT
                      ORIGINAL_FORMAT
                      CRITIC_ACCEPT_FORMAT
                      CRITIC_REJECT_FORMAT
                      CRITIC_HTML_HIGHLIGHT_FORMAT ) );


# Generate const-c.inc and const-xs.inc (using ExtUtils::Constant)

print "Writing const-c.inc and const-xs.inc\n";

WriteConstants( NAME  => $module_name,
                NAMES => [ @constants ] );

# Generate const-defs.inc directly to provide C defines for map enum constants

print "Writing const-defs.inc\n";

open my $const_defs_fh, '>', 'const-defs.inc'
    or die "cannot create const-defs.inc";
foreach my $const (@constants) {
    $const_defs_fh->printf("#define %s %s\n", $const, $const);
}
undef $const_defs_fh;


# Generate the Makefile

my @objects = ( qw( parse_utilities.o
                    parser.o
                    GLibFacade.o
                    writer.o
                    text.o
                    html.o
                    latex.o
                    memoir.o
                    beamer.o
                    man.o
                    opml.o
                    odf.o
                    critic.o
                    XS.o ) );

WriteMakefile(
    NAME         => $module_name,
    VERSION_FROM => 'XS.pm',
    OBJECT       => join(' ', @objects),
    INC          => '-I.',
    );
