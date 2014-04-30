eval 'exec perl -S $0 ${1+"$@"}'
  if 0;#$running_under_some_shell;
# vim:sw=2:tw=69:lines=42:columns=90:fdm=marker:fmr=<<<,>>>

# @(#)$Info: Envv.pm - Perl module to manipulate shell environments. $
#
# @(#)$Copyright: 2001 All rights reserved. $
# @(#)$Email: <dcblack@hldwizard.com>. $

  $main::prog_name = $0;

package Envv;

=head1 NAME

Envv - perl module to aid I<shell independent> manipulation of the environment

=head1 SYNOPSIS

 eval `perl script`

 use Envv;

 use Envv qw{alias setenv export setlocal source
            shell reduce remove splicenv ignorenvv
            envdump envsave envload envpush envpop envdiff
            envvfile envvopt envv module unset unalias};

 use Envv qw{:all};

=head1 PROLOGUE

Tired of having multiple dot files for different programming shells
(B<csh>, B<sh>, B<bash>, B<ksh>, B<zsh>, B<tcsh>, B<tclsh>, B<wish>)?
Tired of keeping multiple dot files (I<.profile>, I<.cshrc>,
I<.bashrc> and I<.tclshrc>) and remembering different syntaxes for
setting environmental variables (B<setenv>, B<export>, B<set>)?
Perhaps this Perl5 module is for you. Now you can write all your
environmental variable settings in your favorite scripting language,
B<PERL>, and use a single file for I<ALL> (*) your shell
environments!

B<*> OK, maybe not all, but at all the ones I am familiar with
including: B<sh>, B<csh>, B<ksh>, B<tcsh>, B<bash>, B<zsh>, B<bash>,
B<tclsh>, and B<wish>. If you let me know of other shells that need
this type of support, I might even add them (quite simple).

Although some would argue for using a single shell, my experience is
that every work environment is different and eventually you have the
fun of importing some tool that uses an alternate shell. Perhaps you
use B<csh> (quite popular) and receive an B<sh> script (common). Or
maybe you want to change over to LINUX where B<bash> is the favorite
shell of the moment (mine too, even if I do use a Macintosh).

You can even use this tool to help with porting!

=head1 USAGE

There are several ways to use B<Envv>.  Usually, you just make each
dot file simply evaluate the STDOUT of your perl script. For most
shells this is simply: eval `YOURSCRIPT`.

The simplest PERL script just uses PERL to set environmental
variables as desired with the addition of 'use Envv;'. When your
script finishes it will automatically dump all the changed variable
settings in the appropriate shell syntax to STDOUT. You can even
'delete $ENV{NAME}' to delete variables.

A slightly more advanced user might employ some of the useful utility 
subroutines in B<Envv> with fairly familiar syntax. These include:

 use Envv qw{setenv export alias remove splicenv reduce};
 setenv    'VARNAME', "VALUE";
 unset     'VARNAME';
 export    "NAME";
 ignorenvv "NAME";
 alias     'NAME','BODY';
 remove    'NAME','VALUE'; # remove VALUE from a list
 splicenv  'NAME',POS,LEN,'VALUE'; # insert VALUE at POS
 reduce    'NAME'; # reduce a list to unique elements
 source    'FILENAME'; # source a file
 module    'SUBCMD', ARGS; # invoke modulecmd

An alternate usage style would be to dump the output of B<Envv> into
a I<FILE>, and then B<source> it. This might be useful to export your
environment to somebody else using a foreign shell (to you). For
example:

 eval 'exec perl -S $0 ${1+"$@"}'
   if $running_under_some_shell;
 use Envv qw{envvopt shell envvfile};
 envvopt all;
 shell 'zsh';
 envvfile "foreign.zsh";

See examples for more ideas.

=head1 EXPORTS

=over

=item B<&setenv>

Sets an environmental variable (i.e. exports it).

 setenv VARNAME,VALUE[,VALUE...];

=item B<&unset>

Removes an environmental variable. 

 unset VARNAME;

Equivalent to 'delete $ENV{VARNAME}'.

=item B<&setlocal>

Sets a local shell variable (i.e. does not export it).

 setlocal VARNAME,VALUE[,VALUE...];

=item B<&reduce>

Reduces a list to its unique elements preserving order on a first come basis.
Assumes environmental variable format in scalar context.

 reduce LIST

 Example 1: scalar &reduce('a:B:c:d:B:e:a') ==> 'a:B:c:d:e'
 Example 2: &reduce(qw(a B c d B e a)) ==> qw(a B c d e)
 Example 3: scalar &reduce('a:B:c','d:B:e:a') ==> 'a:B:c:d:e'

SPECIAL CASE: If only a single non-reference argument is passed, and
it looks like a variable name and $ENV{NAME} exists, it is taken for
for what it appears and pushed onto a list to be reduced just before
environmental variables are exported.

 Example 4: reduce 'PATH'; # deferred until dump time

=item B<&remove>

Removes a VALUE or colon separated list of values from LIST if it exists.

 remove LIST,VALUE

=item B<&splicenv>

Similar to PERL's spice as applied to environmental variables.

 splicenv LIST,INDEX[,LENGTH[,LIST]]

=item B<&alias>

Use B<alias> to create really simple aliases (i.e. no arguments).

 alias NAME,VALUE;

=item B<&ignorenvv>

Use B<ignorenvv> to identify environmental variables that are not
to be output (i.e. ignored). This is usefull with 'envvopt all'.

 ignorenvv; # Deletes the existing ignore list

 ignorenvv NAME[,NAME]; # Add one or more names

=item B<&source>

Use B<source> to 'source' shell files. This is not recommended since
the sourced file must be shell dependent. Use Perl's 'do' or 'require'
instead.

 source FILE;

=item B<&module>

Invokes the public domain 'modules' command.

 module SUBCMD,VALUE[,VALUE...];

=back

=over

=item B<&shell>

Optionally specify the shell to use for formatted output. Default is
to determine the shell of the parent process or use the environmental
variable. Returns the shell type as &envv understands it.

 $shell = shell "/usr/bin/bash";
 if (shell == "sh") { SOMETHING sh-like SPECIFIC }

Currently supports: B<sh>, B<csh>, B<ksh>, B<bash>, B<tcsh>, B<zsh>, 
B<tclsh>, and B<wish>.  Special support for B<vim>, B<perl>, and even
B<Envv> itself.

=item B<$shell>

Interogate $shell to discover the base value to be used by &envv. It is
probably better to use the &shell routine.

 if ($shell eq 'sh') { SOMETHING sh SPECIFIC }

=item B<&envvopt>

Optionall specify envv switches and return result:

 envvopt [reset] [verbose|quiet|debug] [all|changes] [auto|manual];

 'all'     specifies whether to dump all environmental variables.
 'auto'    causes &envv to be called automatically upon exit.
 'changes' only dumps variables that have changed. 
 'debug'   provides additional messages to STDERR.
 'manual'  disables automatic dumping.
 'quiet'   sends nothing to STDERR except error messages.
 'reset'   clears out all the environmental variables and aliases.
 'verbose' duplicates STDOUT to STDERR.

Default is 'quiet changes auto'.

=item B<&envdiff> SH,\@ENV1,\@ENV2

Compares two environment arrays against each other and produces 
a minimal script capable to transforming ENV1 into ENV2.

 @script = envdiff 'modulefile',\pENV1,\pENV2

=item B<&envvfile>

Use this routine to redirect the automatic dumping of &envv to a
specified file. If set to null, automatic dumping is suppressed. If
set to '-', dumping is directed at STDOUT.

 envvfile FILENAME;

=item B<&envload> FILE,\@ARY

Loads environment from FILE into \@ARY. Different file types are
treated differently.

=item B<&envv>

This is the core/final routine that outputs the environment in
various shell formats. 

 envv FILENAME;

=back

=head1 EXAMPLES

 -- EXAMPLE 1 -----------------------------------------------------
 % cat >basic.envv
 #!/usr/local/bin/perl
 # Basic
 use Env; # useful
 use Envv qw{:basic};
 setenv NEWVAR,'somevalue';
 export DEFINEONLY;
 reduce PATH; # simply remove redundancy
 $PATH = remove $PATH,'/oldpath/bin'; # remove a path
 undef $DELETEME; # remove an environmental variable
 delete $ENV{'ZAPME'}; # alternate way to remove
 print STDERR "Only print to STDERR unless redirecting dump";
 ^D
 chmod +x basic.envv
 eval `basic.envv`
 ------------------------------------------------------------------

 -- EXAMPLE 2 -----------------------------------------------------
 % cat >advanced.envv
 #!/usr/local/bin/perl
 # Advanced
 use Env; # useful
 use Envv qw{:all};
 shell 'csh'; # defaults to $SHELL or /etc/passwd
 envvfile 'myfile.csh'; # defaults to STDOUT
 envvopt manual;
 ignorenvv qw{USER SHELL LOGNAME DISPLAY};
 print STDOUT "#!/bin/csh\n"; # printing to STDOUT happens first
 print STDOUT "\n";
 if (shell eq 'sh') {
     print STDOUT 'myfunc() { ls $2 | head -$1 ; } ;',"\n"
 }#endif
 setenv NEWVAR,'somevalue';
 # move or insert /mypath/bin to front
 $PATH = reduce "/mypath/bin:$PATH";
 envv; # send settings to myfile.csh in appropriate format
 ------------------------------------------------------------------

 -- EXAMPLE 3 -----------------------------------------------------
 % cat >perluse.envv
 # Perl usage
 use Envv qw{remove reduce envvopt};
 envvopt manual; # avoid any exporting
 @complex = qw(a B c d B e a);
 @simple = reduce @complex; # becomes (a B c d e)
 @smaller = remove @complex,'B'; # becomes (a c d e a);
 # Perl's builtin splice is more effective for perl
 ------------------------------------------------------------------

=head1 NOTES

If you do send your own stuff to B<STDOUT>, be certain to test the 
results carefully under each shell. Some things do not appear to
B<eval>uate properly under some shells (e.g. B<set -o vi> in bash).
Some things will just have to be put in the dot files with their
own syntax. At least there are not too many of these, and the more
important issues (e.g. the PATH variable) work.

=head1 SEE ALSO

Env is another useful PERL module for environmental variable manipulation.

=head1 AUTHOR

David C Black <dcblack@hldwizard.com>

=cut

# Initialization #<<<1
use strict;
use warnings;
use Sys::Hostname;
use FileHandle;
STDOUT->autoflush(1);
STDERR->autoflush(1);

# Prototypes
sub Escape($@);
sub shell(;@);
sub setlocal($@);
sub export(@);
sub unset(@);
sub setenv($@);
sub prescript(@);
sub postscript(@);
sub sh_type($);
sub sh_rm($$$$$);
sub sh_add($$$$$);
sub is_list($$);
sub envappend($$);
sub envprepend($$);
sub envremove($$);
sub envinsert($$$);
sub envdiff($$$);
sub arrdiff($$$$$);
sub envsave($;$);
sub envdump($$$);
sub envload($;$);
sub envpush;
sub envpop;
sub envvfile($);
sub source($);
sub alias($$);
sub unalias($);
sub ignorenvv(@);
sub reduce(@);
sub remove($$);
sub splicenv($$;$@);
sub echo(@);
sub envvopt(@);
sub envv($);
sub Printf($@);
sub Dump($$@);
sub expand_path($);
sub resolve(@);
sub glob2re(@);
sub find_path(@);
sub relative_path($$);
sub find_dir_containing($$$@);
sub getsoc($@);
sub showdir($);

########################################################################
BEGIN {
    use Exporter   ();
    use Env ('PATH');
    use vars       qw($VERSION @ISA @EXPORT @EXPORT_OK %EXPORT_TAGS @EXPORTS);

    # set the version for version checking
    $VERSION = do { my @r = (q$Revision: 1.7 $ =~ /\d+/g); sprintf "%d."."% 02d" x $#r, @r }; # must be all one line, for MakeMaker
    @ISA         = qw(Exporter);
    @EXPORT      = qw();
    %EXPORT_TAGS = (
        all   => [ qw(&alias &unalias &setenv &unset &export &setlocal
                      &source &prescript &postscript &module &reduce
                      &remove &splicenv &ignorenvv &shell $shell
                      &envv &Printf &envvfile &envvopt &expand_path
                      &is_list
                      &envdiff &envsave &envload &envdump
                      &envprepend &envappend &envinsert &envremove
                      &relative_path &find_path &find_dir_containing
                      &dump &getsoc &showdir) ],
        basic => [ qw{&alias &unalias &setenv &unset &export &setlocal
                      &source &prescript &postscript &module &reduce
                      &remove &splicenv &ignorenvv &shell &expand_path
                      &envprepend &envappend &envinsert &envremove
                      &Printf &relative_path &find_path} ],
        diff  => [ qw{&alias &unalias &setenv &unset &export &setlocal
                      &source &prescript &postscript &module &reduce
                      &remove &splicenv &ignorenvv &shell &expand_path
                      &is_list
                      &envdiff &envsave &envload &envdump
                      &envprepend &envappend &envinsert &envremove
                      &Printf &relative_path &find_path} ],
        setup => [ qw{&alias &unalias &setenv &unset &export &setlocal
                      &source &prescript &postscript &module &reduce
                      &remove &splicenv &ignorenvv &shell &expand_path
                      &Printf &relative_path &find_path &find_dir_containing
                      &getsoc &showdir} ],
    );
    Exporter::export_ok_tags('all');
    Exporter::export_ok_tags('basic');
    Exporter::export_ok_tags('setup');
}#end BEGIN

    use vars      @EXPORT_OK;

    # non-exported package globals
    use vars      qw(
                      $dumpfile
                      $dumpmode
                      $ESCAPE
                      $verbosity
                      $dumpwhat
                    );
    use vars      qw(
                      %ALIAS
                      %IGNORE
                      %LCL
                      %OLD
                      %RED
                      %SHELL
                      $HOME
                      &Escape
                      @ECHO
                      @KNOWN_LIST
                      @KNOWN_NONLIST
                      @MDL
                      @PRE
                      @PST
                      @SRC
                      @UNALIAS
                    );

    # initalize package globals
    $verbosity = 'quiet';
    $dumpwhat  = 'changes';
    $dumpfile  = '-';
    $dumpmode  = 'manual'; # unless import says
    %RED = ();   # list variables to be reduced
    @SRC = ();   # files to be sourced
    @MDL = ();   # module commands
    @PRE = ();   # prescripts
    @PST = ();   # postscripts
    %LCL = ();   # local variables
    %OLD = %ENV; # old environment for comparison
    $HOME = $ENV{'HOME'};
    %SHELL = (
        'bash'     => 'sh',
        'sh'       => 'sh',
        'rsh'      => 'sh',
        'ksh'      => 'sh',
        'zsh'      => 'sh',
        'csh'      => 'csh',
        'tcsh'     => 'csh',
        'tcl'      => 'tcl',
        'wish'     => 'tcl',
        'tclsh'    => 'tcl',
        'vim'      => 'vim',
        'perl'     => 'perl',
        'Envv'     => 'Envv',
    );
    #$ESCAPE = '"!$%^&*()[]<>{}`~|;?\\\'';
    $ESCAPE = q{'\\}; #'
    # Leave the rest for &import

# Subroutines #<<<1
########################################################################

# NOTE: Someday figure out how to use 'import' function. For now we
#       leave it commented out. Idea is to support three functionalities:
#       (1) use Envv; (2) use Envv qw{LIST}; and (3) use Envv ();
#       Auto export would default to on in the first two cases, and
#       in the last case, auto export would default to off.
#
#sub import { #<<<2
#
#    &Exporter::import(@_);
#}
{

    #-------------------------------------------------------------------
    # Figure out what type of shell the parent is
    #-------------------------------------------------------------------
    my $ppid = getppid;
#   if (open(PS,'ps x|')) {
#       my ($line);
#       1 while ($line = <PS> and $line !~ m/^\s*$ppid\s/);
#       close(PS);
#       if ($line =~ m/\((\w+)\)/) {
#           $shell = $1;
#       }#endif
#   }#endif

    #-------------------------------------------------------------------
    # Use environemtal setting if above failed
    #-------------------------------------------------------------------
    $shell = $ENV{'SHELL'} unless defined $shell;
    $shell =~ s:.*/::;

    #-------------------------------------------------------------------
    # Use login information if shell is not valid
    #-------------------------------------------------------------------
    if (not defined $SHELL{$shell}) {
        # Attempt to get it from the passwd file
        $shell = (getpwuid($<))[8];
        $shell =~ s:.*/::;
    }#endif

    die "Envv: $shell not supported!\n" unless defined $SHELL{$shell};

    $dumpmode = 'auto'; # auto export

    1;
}#endsub import

########################################################################
sub Escape($@) { #<<<2
   my ($q,@argv) = @_;
   my ($arg,$c,$str);
   for $arg (@argv) {
      $str = '';
      for $c (split('',$arg)) {
#WAS:    if ($c eq "\\") {
         if ($q eq q(") and $c eq $q) {
            $str .= q("'"'");
         } elsif ($q eq q(') and $c eq $q) {
            $str .= q('"'"');
         } elsif ($q eq q({) and $c eq q({)) {
            $q .= $c;
            $str .= $c;
         } elsif ($q =~ m/{{+/ and $c eq q(})) {
            chop($q);
            $str .= $c;
         } elsif ($q eq q({) and $c eq q(})) {
            $str .= "\\".$c;
         } elsif ($c eq qq(\\)) { # escape escapes
            $str .= "\\".$c;
         } else {
            $str .= $c;
         }#endif
      }#endfor $c
      $arg = $str;
   }#endfor $arg
   return wantarray ? @argv : $argv[0];
}#endsub Escape

########################################################################
sub shell(;@) { #<<<2
    if (@_) {
        $shell = $_[0] if @_;
        $shell =~ s:.*/::;
        die "Envv: $shell not supported!\n" unless defined $SHELL{$shell};
    }#endif
    return $SHELL{$shell};
}#endsub shell

########################################################################
sub setlocal($@) { #<<<2
    my ($var,@val) = @_;
    my $val = join(':',@val);
    chomp($val);
    if ($var =~ m/^\w+$/) {
        print STDERR "setlocal $var '$val'\n" if $verbosity eq 'debug';
        $LCL{$var} = $val;
        eval ('$main::'.$var.' = $val;'); # shadow
    } else {
        print STDERR "ERROR: Setlocal variable '$var' name not legal\n";
    }#endif
}#endsub setlocal

########################################################################
sub export(@) { #<<<2
    my (@var) = @_;
    print STDERR "DEBUG: export @var\n" if $verbosity eq 'debug';
    my ($var,$cmd);
    foreach $var (@var) {
        if ($var =~ m/^\w+$/) {
            no strict;
            $cmd = '{ package main; &Env::import('.$var.'); package Envv; }';
            eval $cmd;
            use strict;
        } else {
            print STDERR "ERROR: Export variable '$var' name not legal\n";
        }#endif
    }#endfor
}#endsub export

########################################################################
# unset VAR - Removes an environmental variable. 
#
sub unset(@) { #<<<2
    my $var;
    foreach $var (@_) {
       if ($var =~ m/^\w+$/) {
           delete $ENV{$var} if defined $ENV{$var};
       } else {
           print STDERR "ERROR: Unset variable '$var' name not legal\n";
       }#endif
    }#endforeach
}#endsub unset

########################################################################
# setenv VAR,VAL - Sets an environmental variable (i.e. exports it).
sub setenv($@) { #<<<2
    my ($var,@val) = @_;
    my $val = '';
    $val = join(':',@val) if @val and defined $val[0];
    my ($cmd);
    chomp($val);
    if ($var =~ m/^\w+$/) {
        print STDERR "DEBUG: setenv $var '$val'\n" if $verbosity eq 'debug';
        no strict;
        $cmd = '{ package main; &Env::import('.$var.'); package Envv; }';
        eval $cmd;
        $cmd = '{ package main; $'.$var.' = '."'".$val."'".'; package Envv; }';
        eval $cmd;
        use strict;
        $ENV{$var} = $val;
    } else {
        print STDERR "ERROR: Setenv variable '$var' name not legal\n";
    }#endif
}#endsub setenv

########################################################################
sub prescript(@) { #<<<2
    my $val;
    foreach $val (@_) {
      push @PRE,$val;
    }#endforeach
}#endsub prescript

########################################################################
sub postscript(@) { #<<<2
    my $val;
    foreach $val (@_) {
      push @PST,$val;
    }#endforeach
}#endsub postscript

########################################################################
# sh_type(elt) - Identifies an sh element. 
# Returns one of VAR, ALIAS, FUNCTION, or UNKNOWN.  In array
# context, returns the identifier and remaining value too.
# LIMITATION: Does not work for any other shell input type.
# Convert to sh if you need to use this function.
sub sh_type($) {
  my @rslt;
  my $elt = shift @_;
  if ($elt =~ m/^(\w+)=/) {
    push @rslt,'VAR',$1,$';
  } elsif ($elt =~ m/^(\w+) [(][)]/) {
    push @rslt,'FUNCTION',$1,$2.$';
  } elsif ($elt =~ m/^alias ([^=]+)=/) {
    push @rslt,'ALIAS',$1,$2;
  } else {
    push @rslt,'UNKNOWN','',$elt;
  }#endif
  return wantarray ? @rslt : $rslt[0];
}#endsub sh_type

########################################################################
# Using the sh_type information, this routine returns an
# "removal" action.
# LIMITATION: Does not work for any other shell. Convert to sh if you
# need to use this function.
sub sh_rm($$$$$) {
  my ($sh, $typ,$nam,$val,$pA) = @_;
  assert(ref $pA eq 'ARRAY');
  if ($sh eq 'modulefile') {
    if ($typ eq 'VAR') {
      push @$pA,"unsetenv $nam";
    } elsif ($typ eq 'FUNCTION') {
      push @$pA,"#undef $nam";
    } elsif ($typ eq 'ALIAS') {
      push @$pA,"#unalias $nam";
    } else { # UNKNOWN
      ; # TODO - Probably an error
    }#endif
  } else { # envv script
    if ($typ eq 'VAR') {
      push @$pA,"unset $nam";
    } elsif ($typ eq 'FUNCTION') {
      push @$pA,"unset -f $nam";
    } elsif ($typ eq 'ALIAS') {
      push @$pA,"unalias $nam";
    } else { # UNKNOWN
      ; # TODO - Probably an error
    }#endif
  }#endif
}#endsub sh_rm

########################################################################
sub sh_add($$$$$) {
  my ($sh, $typ,$nam,$val,$pA) = @_;
  assert(ref $pA eq 'ARRAY');
  if ($sh eq 'modulefile') {
    if ($typ eq 'VAR') {
      push @$pA,"setenv $nam $val";
    } elsif ($typ eq 'FUNCTION') {
      push @$pA,"#undef $nam";
    } elsif ($typ eq 'ALIAS') {
      push @$pA,"alias $nam $val";
    } else { # UNKNOWN
      ; # TODO - Probably an error
    }#endif
  } else { # envv script
    if ($typ eq 'VAR') {
      push @$pA,"setenv \$$nam,$val";
    } elsif ($typ eq 'FUNCTION') {
      push @$pA,"#sub \$$nam,$val";
    } elsif ($typ eq 'ALIAS') {
      push @$pA,"alias \$$nam,$val";
    } else { # UNKNOWN
      ; # TODO - Probably an error
    }#endif
  }#endif
}#endsub sh_add

########################################################################
@KNOWN_LIST = qw(
  PATH
  MANPATH
  CDPATH
  DYLD_LIBRARY_PATH
  LD_LIBRARY_PATH
  LIBRARY_PATH
  LM_LICENSE_FILE
  CDS_LIC_FILE
  INFOPATH
  HELPPATH
  FONTPATH
  PERLLIB
  COMPILER_PATH
);
@KNOWN_NONLIST = qw(
  DISPLAY
);
sub is_list($$) {
  my ($var,$val) = @_;
  return 1 if grep($var eq $_,@KNOWN_LIST);
  return 0 if grep($var eq $_,@KNOWN_NONLIST);
  return ($val =~ m/:/) ? 1 : 0;
}#endsub is_list

########################################################################
# envappend(VAR,VAL) - appends VAL to list VAR
sub envappend($$) {
  my ($var,$val) = @_;
  splicenv $var,-1,0,$val;
}#endsub envappend

########################################################################
# envprepend VAR,VAL) - prepends VAL to list VAR
sub envprepend($$) {
  my ($var,$val) = @_;
  splicenv $var,0,0,$val;
}#endsub envprepend

########################################################################
# envremove(VAR,VAL) - removes VAL from list VAR
sub envremove($$) {
  my ($var,$val) = @_;
  remove $var,$val;
}#endsub envremove

########################################################################
# envinsert(VAR,VAL,POS) - inserts VAL after position POS in list VAR
sub envinsert($$$) {
  my ($var,$val,$pos) = @_;
  splicenv $var,$pos,0,$val;
}#endsub envinsert

########################################################################
# envdiff(SH,ENV1,ENV2) - environment difference
# Compares two environment arrays against each other and produces a
# minimal script capable to transforming ENV1 into ENV2. Environment
# arrays contain bash descriptors of three varieties: variables,
# aliases and functions.  Variables are classified as either scalar
# or lists (colon separated.) Anything else is unknown and ignored
# with a warning. Target shell, SH, is currently either envv (perl)
# or modulefile (TCL). Commands output are limited to:
#
#   MODULEFILE                 ENVV        
#   ----------                 ----------  
#   setenv VAR VAL             setenv VAR,VAL
#   unsetenv VAR               unset VAR   
#   append-path VAR VAL        envappend VAR,VAL
#   prepend-path VAR VAL       envprepend VAR,VAL
#   remove-path VAR VAL        envremove VAR,VAL
#   insert-path VAR VAL POS    envinsert VAR,VAL,POS
#
sub envdiff($$$) {
  my ($sh,$pENV1, $pENV2) = @_;
  assert(ref $pENV1 eq 'ARRAY' and ref $pENV2 eq 'ARRAY');
  my @ENV1 = @$pENV1;
  my @ENV2 = @$pENV2;
  @ENV1 = sort @ENV1;
  @ENV2 = sort @ENV2;
  my ($typ1,$nam1,$val1,$typ2,$nam2,$val2,@envdiff);
  my ($i1,$i2);
  $i1 = $i2 = 0;
  while ($i1 != $#ENV1) {
    ($typ1,$nam1,$val1) = sh_type $ENV1[$i1];
    if ($typ1 eq 'UNKNOWN') {
      ++$i1; # Ignore
      next;
    }#endif
    if ($i2 == $#ENV2) {
      sh_rm $sh,$typ1,$nam1,$val1,\@envdiff; # Extra element in @ENV1 needs deletion
      ++$i1;
      next;
    }#endif
    ($typ2,$nam2,$val2) = sh_type $ENV2[$i2];
    if ($typ2 eq 'UNKNOWN') {
      ++$i2; # Ignore
      next;
    }#endif
    # Compare two elements
    if ($typ1 eq $typ2 and $nam1 eq $nam2) {
      # Resolve conflict
      if (&is_list($nam1,$val1) or &is_list($nam2,$val2)) {
        # Diff lists
        my @L1 = split(':',$val1);
        my @L2 = split(':',$val2);
        &arrdiff($sh,$nam2,\@L1,\@L2,\@envdiff);
        ++$i1;
        ++$i2;
      } else {
        # New value takes precedence
        sh_add($sh,$typ2,$nam2,$val2,\@envdiff);
        ++$i1;
        ++$i2;
      }#endif
    } elsif ($ENV1[$i1] le $ENV2[$i2]) { # advance lower alphabetic element
      sh_rm $sh,$typ1,$nam1,$val1,\@envdiff; # Extra element in @ENV1 needs deletion
      ++$i1;
      next;
    } else {
      # New element in @ENV2 needs creation
      sh_add($sh,$typ2,$nam2,$val2,\@envdiff);
      ++$i2;
    }#endif
  }#endwhile
  while ($i2 != $#ENV2) {
    # Extra element in @ENV2 needs creation
    sh_add($sh,$typ2,$nam2,$val2,\@envdiff);
    ++$i2;
  }#endwhile
  return @envdiff;
}#endsub envdiff

########################################################################
#  Lst1 = a,b,c,d,e,f,g,h,i,j
#  Lst2 = x,y,z,d,e,g,f,h,k,j,l,m,n
#  diff = -a,-b,-c,+(x,y,z),mv f,g,mv k; (l,m,n)+
sub arrdiff($$$$$) {
  my ($sh,$nam,$pL1,$pL2,$pEnvdiff) = @_;
  assert(ref $pL1 eq 'ARRAY' and ref $pL2 eq 'ARRAY' and ref $pEnvdiff eq 'ARRAY');
  my @L1 = @$pL1;
  my @L2 = @$pL2;
  # Classify elements
  my %M; # Map
  V1: for my $v1 (@L1) {
    for my $v2 (@L2) {
      next unless $v1 eq $v2;
      $M{$v1} = 0;
      next V1;
    }#endfor
    --$M{$v1}; # need to delete
  }#endfor
  V2: for my $v2 (@L2) {
    for my $v1 (@L1) {
      next V2 if $v1 eq $v2;
    }#endfor
    ++$M{$v2}; # need to create
  }#endfor
  # Delete elments in L1 not found in L2
  # - watch for begin & end keepers
  my $kb = '';
  my $ke = '';
  for my $i1 (0..$#L1) {
    my $v1 = $L1[$i1];
    $kb = $i1 if $kb eq '' and $M{$v1} == 0;
    $ke = $i1 if $M{$v1} == 0;
    next unless $M{$v1} < 0;
    if ($sh eq 'modulefile') {
      push @$pEnvdiff,"remove-path $nam $v1";
    } else {
      push @$pEnvdiff,"envremove \$$nam,$v1";
    }#endif
  }#endfor
  # Add new elements "appropriately"
  my @pre = ();
  my @app = ();
  my @spl = ();
  my $i1 = -1;
  for my $i2 (0..$#L2) {
    my $v2 = $L2[$i2];
    ++$i1 if $M{$v2} == 0;
    if ($i1 < $kb) { # prepend in reverse order to get correct effect
      if ($sh eq 'modulefile') {
        unshift @pre,"prepend-path $nam $v2";
      } else {
        unshift @pre,"envprepend \$$nam,$v2";
      }#endif
    } elsif ($i1 == $ke) { # insert after
      my $i3 = $#pre+$i1+1;
      if ($sh eq 'modulefile') {
        push @spl,"#insert-path $nam $v2,$i3";
      } else {
        push @spl,"#envinsert \$$nam,$v2,$i3";
      }#endif
    } else { # append
      if ($sh eq 'modulefile') {
        push @app,"append-path $nam $v2";
      } else {
        push @app,"envappend \$$nam,$v2";
      }#endif
    }#endif
  }#endfor
  push @$pEnvdiff,@pre,@spl,@app;
}#endsub arrdiff

########################################################################
# envsave(FILE[,SCRIPT]) - save environment into file
#
my $bash;
$bash = '/bin/bash';
$bash = '/usr/bin/bash'       unless -x $bash;
$bash = '/usr/local/bin/bash' unless -x $bash;
$bash = 'bash'                unless -x $bash;
sub envsave($;$) {
  my ($file,$script) = @_;
  open ENV,">$file" or die "FATAL: Unable to save environment to $file!?\n";
  printf ENV "#!$bash\n";
  printf ENV "# Vers: envv 2.0\n";
  printf ENV "# PID: %s\n",$$;
  printf ENV "# Date: %s\n",scalar localtime;
  close ENV;
  open   BASH,"$bash|";
  printf BASH "source       %s;\n", $script if $script ne '';
  printf BASH "#!$bash     >%s;\n", $file;
  printf BASH "env        >>%s;\n", $file;
  printf BASH "alias      >>%s;\n", $file;
  #rintf BASH "declare -f >>%s;\n", $file;
  close  BASH;
  open ENV,">>$file";
  printf ENV "#EOF\n";
  close ENV;
}#endsub envsave

########################################################################
# envdump(FILE,ENV,NAM) - dump hash with environment into file
#
sub envdump($$$) {
  my ($file,$pEnv,$nam) = @_;
  assert(ref $pEnv eq 'ARRAY');
  # Assuming Perl out
  open ENV,">$file" or die "FATAL: Unable to save environment to $file!?\n";
  printf ENV "#!perl\n";
  printf ENV "# Vers: envv 1.0\n";
  printf ENV "# PID: %s\n",$$;
  printf ENV "# Date: %s\n",scalar localtime;
  printf ENV "%%%s = (\n",$nam;
  foreach my $var (sort keys %$pEnv) {
    my $val = $pEnv->{$var};
    $val =~ s/'/\\'/g;
    printf ENV "  '%s' => '%s'\n",$var,$val;
  }#endforeach
  printf ENV ");#end %%%s\n",$nam;
  close ENV;
}#endsub envdump

########################################################################
# envload(pARY[,FILE]) - load array with environment from file
#
sub envload($;$) {
  my ($pARY,$file) = @_;
  my ($type, $nam);
  if (defined $file and $file ne '') {
    #-------------------------------------------------------------------
    # Examine file to see what it's type is
    die "FATAL: Unable to read $file to restore environment from!?\n" 
      if $file ne '' and not -r $file;
    open ENV,"<$file" or die "FATAL: Unable to read environment file $file!?\n";
    my @ENV = <ENV>;
    close ENV;
    if (scalar @ENV == 0) {
      $type = 'sh';
    } elsif ($ENV[0] =~ m/^#!/) {
      if ($ENV[0] =~ m/\bperl([45])?\b/
      and $ENV[1] =~ m/^# Vers: envv (\d+\.\d+)$/
      and $1 == 1.0
      and $ENV[2] =~ m/^# PID: \d+$/
      and $ENV[3] =~ m/^# Date: /
      and $ENV[$#ENV] =~ m/^[\)];#end [\%]\w+$/
      and $ENV[4] =~ m/^[\%](\w+) = [\(]$/) {
        $type = 'envv1';
        $nam = $1;
      } elsif ($ENV[0] =~ m/\bperl([45])?\b/
           and $ENV[1] =~ m/^# Vers: envv (\d+\.\d+)$/
           and $1 == 2.0
           and $ENV[2] =~ m/^# PID: \d+$/
           and $ENV[3] =~ m/^# Date: /
           and $ENV[$#ENV] =~ m/^#EOF$/
      ) {
        $type = 'envv2';
      } elsif ($ENV[0] =~ m/\bperl\b/) {
        $type = 'perl';
      } elsif ($ENV[0] =~ m/\bt?csh?\b/) {
        $type = 'csh';
      } elsif ($ENV[0] =~ m/\bbash\b/) {
        $type = 'sh';
      } elsif ($ENV[0] =~ m/\bk?sh?\b/) {
        $type = 'sh';
      } else {
        $type = 'UNKNOWN';
      }#endif
    } else {
      $type = 'sh';
    }#endif
  } else {
    $type = 'sh';
  }#endif
  #---------------------------------------------------------------------
  # Load the array
  #---------------------------------------------------------------------
  if ($type eq 'envv') {
    do $file;
    eval "%\$pARY = \\\%1;";
  #---------------------------------------------------------------------
  } elsif ($type eq 'sh') {
    my $tmpf = "/tmp/$$.envv";
    envsave $tmpf,$file;
    my $line;
    open ENV,"<$tmpf";
    while ($line = <ENV>) {
      chomp $line;
      push @$pARY,$line;
    }#endwhile
    close ENV;
    unlink $tmpf;
  #---------------------------------------------------------------------
  } elsif ($type eq 'envv2') {
    while (@ENV) {
      my $line = shift @ENV;
      chomp $line;
      next if $line =~ m/^\s*$/;
      next if $line =~ m/^\s*#.*$/;
      push @$pARY,$line;
    }#endwhile
  #---------------------------------------------------------------------
  } else {
    printf STDERR "ERROR: Unable to load file '%s' of type %s\n",$file,$type;
  }#endif
  #---------------------------------------------------------------------
}#endsub envload

########################################################################
# envpush - save environment to # $HOME/.envv/$shell-$host-$pid-${seq++}.env
our $seq = 0;
sub envpush {
  my $dir = "$HOME/.envv";
  mkdir $dir unless -d $dir;
  $seq++;
  my ($host,$pid) = (hostname,$<);
  my $file = "$HOME/.envv/$shell-$host-$pid-$seq.env";
  envsave $file;
}#endsub envpush

########################################################################
# envpop - restore environment from # $HOME/.envv/$shell-$host-$pid-${--seq}.env
sub envpop {
  my $dir = "$HOME/.envv";
  die "ERROR: Nothing to pop!\n" unless $seq > 0;
  die "ERROR: Somebody deleted $dir!\n" unless -d $dir;
  my ($host,$pid) = (hostname,$<);
  my $file = "$HOME/.envv/$shell-$host-$pid-$seq.env";
  $seq--;
  envload \$ENV,$file;
}#endsub envpop

########################################################################
sub envvfile($) {
    my ($file) = @_;
    if ($file =~ m:^[-.\w/]*$:) {
        $dumpfile = $file;
    } else {
        printf STDERR "ERROR: Illegal dump file specification '%s' - IGNORED\n",$file;
    }#endif
    $dumpmode = 'manual' unless $dumpfile ne '';
}#endsub envvfile

########################################################################
sub source($) { #<<<2
    my ($file) = @_;
    if (-r $file) {
       push @SRC,$file;
    } else {
       printf STDERR "ERROR: Unable to source unreadable file '%s' - IGNORED\n",$file;
    }#endif
}#endsub source

########################################################################
BEGIN { #<<<2
  $ENV{MODULESHOME} = '' unless exists $ENV{MODULESHOME};
  my $init = "$ENV{MODULESHOME}/init/perl";
  sub module {
    eval `$ENV{MODULESHOME}/bin/modulecmd perl @_`;
  }
}#endsub BEGIN

########################################################################
sub alias($$) { #<<<2
    my ($name,$body) = @_;
    $ALIAS{$name} = $body;
}#endsub alias

########################################################################
sub unalias($) { #<<<2
    my ($name) = @_;
    push @UNALIAS, $name;
}#endsub unalias

########################################################################
sub ignorenvv(@) { #<<<2
    if (@_ == 0) {
        %IGNORE = ();
        return 0;
    }#endif
    my ($var,$cnt);
    foreach $var (@_) {
        next if defined $IGNORE{$var};
        $IGNORE{$var} = 1;
        $cnt++;
    }#endforeach
    return $cnt;
}#endsub ignorenvv

########################################################################
# Reduce an array to its unique elements keeping the first found.
# Assumes environmental variable format in scalar context.
#
#   Example 1: scalar &reduce('a:B:c:d:B:e:a') ==> 'a:B:c:d:e'
#   Example 2: &reduce(qw(a B c d B e a)) ==> qw(a B c d e)
#   Example 3: scalar &reduce('a:B:c','d:B:e:a') ==> 'a:B:c:d:e'
#
# SPECIAL CASE: If only a single non-reference argument is passed, and
# it looks like a variable name and $ENV{NAME} exists, it is taken for
# for what it appears and pushed onto a list to be reduced
# just before environmental variables are exported.
#
#   Example 4: reduce 'PATH'; # deferred until dump time
#-----------------------------------------------------------------------
sub reduce(@) { #<<<2
    my (@old) = @_;
    my (%old,@new,$old);
    if (scalar @old == 1 and $old[0] =~ m/^\w+/ and defined $ENV{$old[0]}) {
        $old = shift(@old);
        $RED{$old} = 1;
        return $old; # just in case
    }#endif
    @old = split(':',join(':',@old)) unless wantarray;
    # Create a list of unique entries
    foreach $old (@old) {
        $old =~ s://+:/:;
        $old{$old} = 1;
    }#endforeach
    @new = ();
    foreach $old (@old) {
        next unless defined $old{$old} and $old ne '';
        push(@new,$old);
        delete $old{$old};
    }#endforeach
    return wantarray ? @new : join(':',@new);
}#endsub reduce

########################################################################
# Remove an one or more elements from an array as in set subtraction.
# Assumes environmental variable format in scalar context. In this
# context, the value is considered to be the last element and may itself
# be a list of elements to be removed. In array context there are two
# choices: A) a single element, or B) an array reference (anonymous is
# fine). If you really need to remove an array reference from an array
# of references, create an anonymous array containing the array reference.
#   Example 1: scalar &remove('a:B:c:d:B:e:a','B') ==> 'a:c:d:e:a'
#   Example 2: scalar &remove('a:B:c:D:B:e:a','B:D') ==> 'a:c:e:a'
#   Example 3: &remove(qw(a B c d B e a),'B') ==> qw(a c d e a)
#   Example 4: &remove(qw(a B c D B e a),[qw{B D}]) ==> qw(a c e a)
#-----------------------------------------------------------------------
sub remove($$) { #<<<2
    my (@old) = @_;
    my (@val,$val,$old,@new);
    push(@val,pop(@old));
    if (wantarray) {
        @val = @{$val[0]} if ref($val[0]) eq 'ARRAY';
    } else {
        @val = split(':',$val[0]);
        @old = split(':',join(':',@old));
    }#endif
    @new = ();
    OLD: foreach $old (@old) {
        foreach $val (@val) {
            next OLD if $old eq $val;
        }#endforeach
        push @new,$old;
    }#endforeach
    return wantarray ? @new : join(':',@new);
}#endsub remove

########################################################################
# Splice environmental variables in the same way perl splice does with
# convention that colon (:) separates list elements. First argument may
# be either a reference or the name of an environmental variable.
#
# SYNOPSIS
#   $removed = splicenv  $VAR,$OFFSET[,$LENGTH[,@LIST]];
#   $removed = splicenv \$VAR,$OFFSET[,$LENGTH[,@LIST]];
#   $removed = splicenv \@VAR,$OFFSET[,$LENGTH[,@LIST]];
#   @removed = splicenv \$VAR,$OFFSET[,$LENGTH[,@LIST]];
#   @removed = splicenv \@VAR,$OFFSET[,$LENGTH[,@LIST]]; # identical to splice
# Returns joined environmental variable in scalar context; otherwise,
# returns array. Similarly for passed variable.
sub splicenv($$;$@) { #<<<2
  my ($varref,$offset,$length,@lst) = @_;
  my (@var,@rem);
  no strict;
  if (not ref($varref)) {
    $varref = \$ENV{$varref};
  }#endif
  if (ref($varref) eq 'SCALAR') {
    if (defined $$varref) {
      @var = split(':',$$varref);
    } else {
      @var = ();
    }#endif
  } else {
    @var = @$varref;
  }#endif
  if (@lst) { 
    $offset = 0 if $offset > $#var;
    printf STDERR "DEBUG: splice (@var),$offset,$length,(@lst)\n" if $verbosity eq 'debug';
    @rem = splice(@var,$offset,$length,@lst);
  } elsif ($length ne '') {
    @rem = splice(@var,$offset,$length);
  } else {
    @rem = splice(@var,$offset);
  }#endif
  printf STDERR "DEBUG: splicenv (@rem)(@var)\n" if $verbosity eq 'debug';
  if (ref($varref) eq 'SCALAR') {
    $$varref = join(':',@var);
  } else {
    @$varref = @var;
  }#endif
  use strict;
  return wantarray ? @rem : join(':',@rem);
}#endsub splicenv

########################################################################
sub echo(@) { #<<<2
  push @ECHO,sprintf("%s\n",join(' ',@_));
}#endsub echo

########################################################################
sub envvopt(@) { #<<<2
    my ($val);
    for $val (@_) {
        if (grep($val eq $_,qw{all changes})) {
            $dumpwhat = $val;
        } elsif (grep($val eq $_,qw{verbose quiet debug})) {
            $verbosity = $val;
        } elsif (grep($val eq $_,qw{auto manual})) {
            $dumpmode = $val;
        } elsif ($val eq 'reset') {
            @SRC = ();
            @MDL = ();
            %RED = ();
            %ENV = %OLD;
            %LCL = ();
        }#endif
    }#endfor
    return "$dumpwhat $verbosity $dumpmode";
}#endsub envvopt

########################################################################
sub envv($) { #<<<2
    my ($file) = @_;
    my ($var,$val);
    open(SHFILE,">$file");

    #-------------------------------------------------------------------
    # Reduce specified variables
    #-------------------------------------------------------------------
    for $var (keys %RED) {
        printf STDERR "INFO: reducing $var\n" if $verbosity eq 'debug';
        $ENV{$var} = reduce $ENV{$var};
    }#endfor

    #-------------------------------------------------------------------
    # Delete removed variables
    #-------------------------------------------------------------------
    printf STDERR "INFO: Deleting removed variables\n" if $verbosity eq 'debug';
    for $var (sort keys %OLD) {
        next if defined $ENV{$var};
        if (shell eq 'csh') {
            printf SHFILE qq{unsetenv %s;\n},$var;
            printf STDERR qq{unsetenv %s;\n},$var if $verbosity eq 'verbose';
        } elsif (shell eq 'tcl') {
            printf SHFILE "unset env(%s)\n",$var;
            printf STDERR "unset env(%s)\n",$var if $verbosity eq 'verbose';
        } elsif (shell eq 'vim') {
            printf STDERR qq{Unable to unset env(%s)\n},$var if $verbosity eq 'verbose';
        } elsif (shell eq 'perl' or shell eq 'Envv') {
            printf SHFILE "delete \$ENV{%s};\n",$var;
            printf STDERR "delete \$ENV{%s};\n",$var if $verbosity eq 'verbose';
        } else {
            printf SHFILE qq{unset %s;\n},$var;
            printf STDERR qq{unset %s;\n},$var if $verbosity eq 'verbose';
        }#endif
    }#endfor

    #-------------------------------------------------------------------
    # Redefine new or changed variables
    #-------------------------------------------------------------------
    printf STDERR "INFO: Setting environmental variables\n" if $verbosity eq 'debug';
    for $var (sort keys %ENV) {
        next if $IGNORE{$var};
        next if $dumpwhat ne 'all' and defined $OLD{$var} and $OLD{$var} eq $ENV{$var};
        next unless $var ne ''; # workaround funky bug
        if (shell eq 'csh') {       ## C-shell
            $val = Escape(q{"},$ENV{$var});
            printf SHFILE qq{setenv %s "%s" ;\n},$var,$val;
            printf STDERR qq{setenv %s "%s" ;\n},$var,$val if $verbosity eq 'verbose';
        } elsif (shell eq 'tcl') {  ## TCL
            $val = Escape(q({),$ENV{$var});
            printf SHFILE "set env(%s) {%s}\n",$var,$val;
            printf STDERR "set env(%s) {%s}\n",$var,$val if $verbosity eq 'verbose';
        } elsif (shell eq 'vim') {  ## VIM
            $val = Escape(q{"},$ENV{$var});
            printf SHFILE qq{let \$%s = "%s"\n},$var,$val;
            printf STDERR qq{let \$%s = "%s"\n},$var,$val if $verbosity eq 'verbose';
        } elsif (shell eq 'perl') { ## PERL
            $val = Escape(q{'},$ENV{$var});
            printf SHFILE qq{\$ENV{%s} = '%s';\n},$var,$val;
            printf STDERR qq{\$ENV{%s} = '%s';\n},$var,$val if $verbosity eq 'verbose';
        } elsif (shell eq 'Envv') { ## ENVV
            $val = Escape(q{'},$ENV{$var});
            printf SHFILE qq{setenv '%s', '%s';\n},$var,$val;
            printf STDERR qq{setenv '%s', '%s';\n},$var,$val if $verbosity eq 'verbose';
        } else {                    ## SH BASH
            $val = Escape(q{"},$ENV{$var});
            printf SHFILE qq{%s="%s"; export %s ;\n},$var,$val,$var;
            printf STDERR qq{%s="%s"; export %s ;\n},$var,$val,$var if $verbosity eq 'verbose';
        }#endif
    }#endfor

    #-------------------------------------------------------------------
    # Handle echoes
    #-------------------------------------------------------------------
    printf STDERR "INFO: echoing messages\n" if $verbosity eq 'debug';
    for $val (@ECHO) {
        if (shell eq 'csh') {
            $val = Escape(q("),$val);
            printf SHFILE qq{echo "%s" ;\n},$val;
            printf STDERR qq{echo "%s" ;\n},$val if $verbosity eq 'verbose';
        } elsif (shell eq 'tcl') {
            $val = Escape("\173",$val);
            printf SHFILE "puts stderr {%s}\n",$val;
            printf STDERR "puts stderr {%s}\n",$val if $verbosity eq 'verbose';
        } elsif (shell eq 'vim') {
            $val = Escape(q("),$val);
            printf SHFILE qq{echo "%s"\n},$val;
            printf STDERR qq{echo "%s"\n},$val if $verbosity eq 'verbose';
        } elsif (shell eq 'perl') {
            $val = Escape(q("),$val);
            printf SHFILE qq{printf STDERR "%s";\n},$val;
            printf STDERR qq{printf STDERR "%s";\n},$val if $verbosity eq 'verbose';
        } elsif (shell eq 'Envv') {
            $val = Escape(q('),$val);
            printf SHFILE qq{echo '%s', '%s';\n},$val;
            printf STDERR qq{echo '%s', '%s';\n},$val if $verbosity eq 'verbose';
        } else { # sh
            $val = Escape(q("),$val);
            printf SHFILE qq{echo "%s";\n},$val;
            printf STDERR qq{echo "%s";\n},$val if $verbosity eq 'verbose';
        }#endif
    }#endfor
    #-------------------------------------------------------------------
    # Handle local variables
    #-------------------------------------------------------------------
    printf STDERR "INFO: Setting local variables\n" if $verbosity eq 'debug';
    for $var (sort keys %LCL) {
        if (shell eq 'csh') {
            $val = Escape(q("),$LCL{$var});
            printf SHFILE qq{set %s = "%s" ;\n},$var,$val;
            printf STDERR qq{set %s = "%s" ;\n},$var,$val if $verbosity eq 'verbose';
        } elsif (shell eq 'tcl') {
            $val = Escape(q({),$LCL{$var});
            printf SHFILE "set %s {%s}\n",$var,$val;
            printf STDERR "set %s {%s}\n",$var,$val if $verbosity eq 'verbose';
        } elsif (shell eq 'vim') {
            $val = Escape(q("),$LCL{$var});
            printf SHFILE qq{let %s = "%s"\n},$var,$val;
            printf STDERR qq{let %s = "%s"\n},$var,$val if $verbosity eq 'verbose';
        } elsif (shell eq 'perl') {
            $val = Escape(q('),$LCL{$var});
            printf SHFILE qq{\$%s = '%s';\n},$var,$val;
            printf STDERR qq{\$%s = '%s';\n},$var,$val if $verbosity eq 'verbose';
        } elsif (shell eq 'Envv') {
            $val = Escape(q('),$LCL{$var});
            printf SHFILE qq{setlocal '%s', '%s';\n},$var,$val;
            printf STDERR qq{setlocal '%s', '%s';\n},$var,$val if $verbosity eq 'verbose';
        } else { # sh
            $val = Escape(q("),$LCL{$var});
            printf SHFILE qq{%s="%s";\n},$var,$val;
            printf STDERR qq{%s="%s";\n},$var,$val if $verbosity eq 'verbose';
        }#endif
    }#endfor

    #-------------------------------------------------------------------
    # Sources
    #-------------------------------------------------------------------
    for $val (@SRC) {
        printf STDERR "INFO: Sourcing $val\n" if $verbosity eq 'debug';
        if (shell eq 'csh') {
            printf SHFILE qq{source %s;\n},$val;
            printf STDERR qq{source %s;\n},$val if $verbosity eq 'verbose';
        } elsif (shell eq 'tcl') {
            printf SHFILE "source %s\n",$val;
            printf STDERR "source %s\n",$val if $verbosity eq 'verbose';
        } elsif (shell eq 'vim') {
            printf SHFILE "source! %s\n",$val;
            printf STDERR "source! %s\n",$val if $verbosity eq 'verbose';
        } elsif (shell eq 'perl' or shell eq 'Envv') {
            printf SHFILE "do '%s';\n",$val;
            printf STDERR "do '%s';\n",$val if $verbosity eq 'verbose';
        } else {
            printf SHFILE qq{. %s;\n},$val;
            printf STDERR qq{. %s;\n},$val if $verbosity eq 'verbose';
        }#endif
    }#endforr $val (@SRC)


    #-------------------------------------------------------------------
    # Handle aliases
    #-------------------------------------------------------------------
    printf STDERR "INFO: Unsetting aliases\n" if scalar(@UNALIAS) > 0 and $verbosity eq 'debug';
    for $var (@UNALIAS) {
        if (shell eq 'csh') {       ## C-SHELL
            printf SHFILE qq{unalias %s ;\n},$var;
            printf STDERR qq{unalias %s ;\n},$var if $verbosity eq 'verbose';
        } elsif (shell eq 'tcl') {  ## TCL/TK
            $val =~ s{\\!\*}{\$args}g;
            printf SHFILE "rename %s delete\n",$var;
            printf STDERR "rename %s delete\n",$var if $verbosity eq 'verbose';
        } elsif (shell eq 'vim') {  ## VIM
            printf STDERR "WARNING: unalias %s not supported in %s\n",$var,$shell;
        } elsif (shell eq 'perl') { ## PERL
            printf STDERR "WARNING: unalias %s not supported in %s\n",$var,$shell;
        } elsif (shell eq 'Envv') { ## ENVV
            printf SHFILE "unalias '%s';\n",$var,$val;
            printf STDERR "unalias '%s';\n",$var,$val if $verbosity eq 'verbose';
        } else {                    ## SH/BASH
            if ($val =~ m/\\!/) { ## SH FUNCTION
               printf SHFILE qq{unset -f %s ;\n},$var;
               printf STDERR qq{unset -f %s ;\n},$var if $verbosity eq 'verbose';
            } else {                ## SH ALIAS
               printf SHFILE qq{unalias %s ;\n},$var;
               printf STDERR qq{unalias %s ;\n},$var if $verbosity eq 'verbose';
            }#endif
        }#endif
    }#endfor
    printf STDERR "INFO: Setting aliases\n" if scalar(keys %ALIAS) > 0 and $verbosity eq 'debug';
    for $var (sort keys %ALIAS) {
        if (shell eq 'csh') {
            $val = Escape(q('),$ALIAS{$var});
            printf SHFILE qq{alias %s '%s' ;\n},$var,$val;
            printf STDERR qq{alias %s '%s' ;\n},$var,$val if $verbosity eq 'verbose';
        } elsif (shell eq 'tcl') {
            $val = Escape(q({),$ALIAS{$var});
            $val =~ s{\\!\*}{\$args}g;
            printf SHFILE "proc %s {args} { %s }\n",$var,$val;
            printf STDERR "proc %s {args} { %s }\n",$var,$val if $verbosity eq 'verbose';
        } elsif (shell eq 'vim') {
            ; # Aliases not supported
            $val = Escape(q("),$ALIAS{$var});
            printf STDERR "WARNING: alias %s not supported in %s\n",$var,$shell;
        } elsif (shell eq 'perl') {
            $val = $ENV{$var};
            $val =~ s{\\!\*}{\@ARGV}g;
            $val =~ s{\\!\\!(\d+):}{\$ARGV[$1-1]}g;
            printf SHFILE "sub %s { %s }\n",$var,$val;
            printf STDERR "sub %s { %s }\n",$var,$val if $verbosity eq 'verbose';
        } elsif (shell eq 'Envv') {
            $val = Escape(q('),$ALIAS{$var});
            printf SHFILE "alias '%s', '%s';\n",$var,$val;
            printf STDERR "alias '%s', '%s';\n",$var,$val if $verbosity eq 'verbose';
        } else {
            $val = Escape(q('),$ALIAS{$var});
            if ($val =~ m/\\!\*/) {
               $val =~ s{\\!\*}{$Envv::dlr@}g;
               $val =~ s{\\!\\!:}{\$}g;
               $val =~ s/'"'"'/'/g;
               $val .= ' ;' unless $val =~ m/[\&]\s*$/;
               printf SHFILE qq{function %s { %s } ;\n},$var,$val;
               printf STDERR qq{function %s { %s } ;\n},$var,$val if $verbosity eq 'verbose';
            } else { #simple
               printf SHFILE qq{alias '%s'='%s' ;\n},$var,$val;
               printf STDERR qq{alias '%s'='%s' ;\n},$var,$val if $verbosity eq 'verbose';
            }#endif
        }#endif
    }#endfor

    #-------------------------------------------------------------------
    # Postscripts
    #-------------------------------------------------------------------
    printf STDERR "INFO: Handling postscripts\n" if $verbosity eq 'debug';
    for $val (@PST) {
        printf STDERR "INFO: $val\n" if $verbosity eq 'debug';
        printf SHFILE qq{%s;\n},$val;
        printf STDERR qq{%s;\n},$val if $verbosity eq 'verbose';
    }#endfor $val (@SMDL)

    close(SHFILE);
}#endsub envv

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sub Printf($@) { #<<<2
  my ($tmp,$cnt);
  my @vars = ();
  my $fmt = '';
  while (@_) {
    $fmt .= ($tmp = shift @_);
    ($tmp = $fmt) =~ s/%%//g;
    $cnt = ($tmp =~ s/%-?\d*\w//g);
    printf STDERR "ERROR: Printf requires %d args, but only has %d\n",$cnt,scalar @_
      unless $cnt <= scalar @_;
    push @vars,splice(@_,0,$cnt,());
  }#endwhile
  printf $fmt,@vars;
}#endsub Printf

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# sub Dump $INDEX,$TITLE,@VARS #<<<2
sub Dump($$@) {
   # Dump key internal structures
   use Dumpvalue;
   my ($M,$title,@VARS) = @_;
   my $SEP = '-' x 78;
   $M = ' '.$M if $M ne '';
   printf STDERR "%-78.78s\n\n","DUMP($main::prog_name$M): $title ".$SEP;
   my $dumper = new Dumpvalue(compactDump => 1);
   select STDERR;
   $dumper->dumpvars('main',@VARS);
   select STDOUT;
   printf STDERR "\n%-78.78s\n",$SEP;
}#endsub Dump

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sub expand_path($) { #<<<2
  # Return the real pathname of a file (without ./, ../ and symbolic links)
  use File::Basename;
  my ($path) = @_;
  # Remove meaningless garbage
  $path =~ s/\/+/\//g;    # foo//bar  => foo/bar
  $path =~ s/\/\.\//\//g; # foo/./bar => foo/bar
  $path =~ s/\/$//;       # remove trailing /
  # Don't bother with non-existant paths
  if (not -e $path) {
#   Warn "No such path $path";
    return $path;
  }#endif
  my $dir = $path;
  my $file = '';
  ($file,$dir) = &fileparse($path) unless -d $path;
  if (not -d $dir) {
#   Warn "No such directory $dir";
    return $path;
  }#endif
  use Cwd;
  my $here = getcwd();
  chdir $dir;
  $dir = getcwd();
  chdir $here;
  $dir =~ s/\/+$//;
  return $file ? "$dir/$file" : $dir;
}#endsub expand_path
###############################################################################
sub resolve(@) { #<<<2
  my ($path, $max, $link, $dir, @result);
  PATH: foreach $path (@_) {
    $max = 10;
    $link = $path;
    while (-l $path and $max-- > 0) {
      $link = readlink $path;
      last if $link =~ m{^/};
      $path =~ s#/[^/]+#/#;
      $link = $path.$link;
    }#endwhile
    $path = $link;
    push @result,$path;
  }#endforeach
  return wantarray ? @result : $result[0];
}#endsub resolve
###############################################################################
sub glob2re(@) { #<<<2
   foreach my $glob (@_) {
      next unless $glob =~ m/[*?]/;
      $glob =~ s/[.]/\\./g;
      $glob =~ s/[*]/.*/g;
      $glob =~ s/[?]/./g;
   }#endforeach
   return wantarray ? @_ : "@_";
}#endsub glob2re
###############################################################################
sub find_path(@) { #<<<2
  # USAGE: $file = find_path qw(OPTIONS FILE...);
  # OPTIONS: [-a][-l][-d DIR][-v VAR]
  my @args = @_;
  my (%OPT,@dirs,@found,$elt);
  ELT: while (@args) {
    $elt = shift(@args);
    next ELT if $elt eq '';
    if ($elt =~ m/^-\w+/) {
      # Handle options
      if ($elt =~ m/^-[alx]$/) {
        $OPT{$elt} = 1;
      } elsif ($elt =~ m/^(-d)(\d*)$/) {
        $OPT{$1} = $2;
        $OPT{$elt} = 1 if $elt eq '-d';
      } elsif ($elt eq '-v') {
        my $var = shift(@args);
        push @dirs,split(':',$ENV{$var});
      }#endif
      next ELT;
    } elsif ($elt =~ m:^/:) {
      # Handle full path
      if (not -d $elt) {
        $elt = &resolve($elt) if $OPT{'-l'};
        push(@found, $elt);
      }#endif
    } else {
       DIR: foreach my $dir (@dirs) {
         GLOB: foreach my $eltpath (glob "$dir/$elt") {
           next GLOB unless not -d $eltpath;
           $eltpath = &resolve($eltpath) if $OPT{'-l'};
           push(@found, $eltpath);
           last DIR unless $OPT{'-a'};
         }#endforeach
       }#endforeach
    }#endif
  }#endforeach
  if ($OPT{'-d'} > 0) {
    foreach my $found (@found) {
      $found =~ s/(\/[^\/]+){$OPT{'-d'}}$//;
    }#endforeach
  }#endif
  return wantarray ? @found : $found[0];
}#endsub find_path
###############################################################################
sub relative_path($$) { #<<<2
  # Figure out common elements of two paths
  my ($fm,$to) = @_;
  #----------------------------------------------------------------------------
  #  Fm              To              Result      #fm #to Min Com
  #  --              --              ------      --- --- --- ---
  #  /a/b/c/         /d/e/y.txt      /d/e/y.txt   3   2   1  -1
  #  /a/b/c/         /a/b/c/y.txt    ./y.txt      3   3   2   2
  #  /a/b/c/         /a/b/c/d/y.txt  ./d/y.txt    3   4   2   2
  #  /a/b/c/         /a/b/d/y.txt    ../y.txt     3   3   2   1
  #  /a/b/c/d/x.txt  /a/b/y.txt      ../../x.txt  4   2   1   1
  #----------------------------------------------------------------------------
  $fm = basename($fm) unless -d $fm;
  $fm = &Real_path($fm);
  $to = &Real_path($to);
  if (substr($fm,0,1) ne '/') {
#   Warn "No such path $fm";
    return $to;
  }#endif
# if (substr($to,0,1) eq '/') {
#   Warn "No such path $to";
#   return $to;
# }#endif
  use File::Basename;
  my $file;
  ($file,$to) = &fileparse($to);
  chop $to; # remove trailing /
  my @fm = split(m!\/+!,$fm);
  my @to = split(m!\/+!,$to);
  my $min = ($#fm < $#to) ? $#fm : $#to;
  my $common = -1;
  for my $i (0..$min) {
    last if $fm[$i] ne $to[$i];
    $common++;
  }#endfor
  if ($common < 1) {
    return "$to/$file";
  }#endif
  my @result = @to[($common+1)..$#to];
  if ($common < $#fm) {
    unshift @result,('..') x ($#fm - $common);
  } else {
    unshift @result,'.';
  }#endif
  return join('/',@result,$file);
}#endsub relative_path
# find_dir_containing(...) ################################################<<<2
# Usage: ($hits,@found)
#          = &find_dir_containing(
#              $from_dir, # where you want to start looking from
#              $min_hits, # minimum number of sub_dirs to be found
#              $maxdepth, # maximum find search depth (to limit search time)
#              @sub_dirs  # required subdirectories
#            );
# DESCRIPTION: Attempt to find a directory containing two or more of
#              the indicated subdirectories within or below $from_dir.
our $dlr = '$';
our (%FOUND, $HITS, @LIST, $MAXDEPTH);
sub find_dir_containing($$$@) {
  my $root     = shift @_;
  my $min_hits = shift @_;
  my $maxdepth = shift @_;
  @LIST = @_;
  $HITS = 0;
  %FOUND = ();
  # First check to see if we are possible within a sub-directory
  # Climb tree (go almost to the top)
  # NOTE: We stop at the first successful match
  my $found = $root;
  while ($found =~ m!^(?:/[^/]+){2}!) {
    # Make sure minimum hits satisfied
    $HITS = scalar grep(-d "$found/$_",@LIST);
    if ($HITS >= $min_hits) {
      $FOUND{$found} = $HITS;
      return ($HITS,$found);
    } else {
      $HITS = 0;
    }#endif
    # Move up tree by one directory
    $found =~ s{/+[^/]+/?$}{};
  }#endwhile
  # Attempt more exhaustive search downwards
  use File::Find ();
  use vars qw/*name *dir *prune/;
  sub contains_list {
    # Find a parent directory no deeper than $MAXDEPTH
    # whose name is in @LIST
    return 0 unless -d _;
    my $file = $_;
    my $dir = $File::Find::dir;
    $dir =~ s/\/$//;
    my @tmp = split(m{/},$dir);
    my $depth = scalar @tmp;
    if ($depth > $MAXDEPTH) {
       $File::Find::prune = 1;
       return 0;
    }#endif
    return 0 unless grep($_ eq $file,@LIST);
    $FOUND{$dir}++;
    return ++$HITS;
  }#endsub contains_list
  my @tmp = split(m{/},$root);
  $MAXDEPTH = scalar(@tmp) + $maxdepth;
  File::Find::find({wanted => \&contains_list}, $root);
  return ($HITS,sort keys %FOUND);
}#endsub find_dir_containing

###############################################################################

# Find <<<2
#-----------------------------------------------------------------------
# The following is a special search routine that attempts to establish
# the $SOC environmental variable.
#-----------------------------------------------------------------------
#
use vars qw($REQD @found);
#
# ASSERTIONS
#    Either:
#    (A) We are above the project hierarchy
#    (B) We are within the project hierarchy
#    In any case we must be below the SOC level.
#    Also, we assume existance of the following 
#    minimal directory:
#    <SOC>/<IP>/workarea -- contains shared resources for the IP
#
# Search for the 'workarea' directory either downward or at each
# directory above us.

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Following is a cool Perl module that emulates the UNIX
# find command only faster and in Perl!
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
use File::Find;

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Following subroutine is used by find.pl
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sub wanted {
   my ($dev,$ino,$mode,$nlink,$uid,$gid);
   if (/^$Envv::REQD$/ &&
      (($dev,$ino,$mode,$nlink,$uid,$gid) = lstat($_)) &&
      -d _) 
   {
      push @Envv::found, "$File::Find::dir";
   }#endif
   return 1;
}#endsub wanted

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Following routines for for SOC environments
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
use Cwd;
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# USAGE: ($soc,$ip) = getsoc(ANCHOR_PATH, REQUIRED_DIR(S)); #<<<2
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sub getsoc($@) {
   my $ANCHOR_PATH = '('.shift(@_).')';
   while ($_[0] =~ m#^/#) {
      $ANCHOR_PATH .= '|('.shift(@_).')';
   }#endwhile
   my @REQD = @_;
   my $REQD = '('.join(')|(',@REQD).')';
   #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   # ASSERTIONS
   #    Either:
   #    (A) We are above the project hierarchy
   #    (B) We are within the project hierarchy
   #    In any case we must be below the SOC level.
   #    Also, we assume existance of the following 
   #    minimal directory:
   #    <ANCHOR_PATH>/.../<SOC>/<IP>/$REQD is always present in a valid path
   #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   @Envv::found = ();
   my $dir = &getcwd;
   #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   my @f;
   while ($dir ne '') {
      last if ($dir !~ m:$ANCHOR_PATH:); # we're not even near the target
      if ($dir =~ m:/$REQD$:) {
         push @Envv::found,$`;
      } elsif (@f = grep(-d "$dir/$_",@REQD)) {
         push @Envv::found,$dir;
      }#endif
      last if @Envv::found;
      #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      # Else we didn't find it, so look upwards...
      #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      # Strip off last /<DIR> from path (i.e. move upward)
      $dir =~ s:/[^/]+$::;
   }#endwhile
   # Do we need to do it the hard way?
   if (scalar(@Envv::found) < 1 and $dir =~ m:$ANCHOR_PATH:) {
      $dir = &getcwd;
      while ($dir ne '') {
         printf STDERR "Searching %s\n", $dir;
         $Envv::REQD = $REQD;
         &find($dir);
         #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         # Else we didn't find it, so look upwards...
         #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         # Strip off last /<DIR> from path (i.e. move upward)
         $dir =~ s:/[^/]+$::;
      }#endwhile
   }#endif
   # Now let's see if we found it
   if (scalar(@Envv::found) < 1) { # too few
      printf STDERR "WARNING: Not in a project shadow (ie. under %s/...)!\007\n",$ANCHOR_PATH;
      return wantarray ? ('', '') : '';
   } elsif (scalar(@Envv::found) > 1) { # too many
      printf STDERR "WARNING: Need to move down into a project shadow!\007\n";
      return wantarray ? ('', '') : '';
   }#endif

   # Yea! We found it... now to compute the SOC and IP names

   my ($soc, $ip);
   $soc = shift(@Envv::found);
   $soc =~ s:/([^/]+)$::;
   $ip  = $1;
   return wantarray ? ($soc, $ip) : $soc;
}#endsub getsoc

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# USAGE: showdir; #<<<2
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sub showdir($) {
   my ($wd) = @_;
   $wd = &getcwd if $wd eq '';
   my $user = getlogin || (getpwuid($<))[0] || "Intruder!";
   $wd =~ s{^/home/$user/}{~$1/};
   $wd =~ s{^/home/(\w+)/}{~$1/};
   $wd =~ s{/user/$user/}{/~/};
   $wd =~ s{^/}{};
   while (length($wd) > 10 and index($wd,'/') > 0) {
      $wd =~ s:^[^/]+/::;
   }#endwhile
   my $banner = `header $wd`;
   return wantarray ? ($banner, $wd) : $banner;
}#endsub showdir

sub END { #<<<2
    &envv($dumpfile) unless $dumpmode ne 'auto';
}#endsub END

1;

__END__
