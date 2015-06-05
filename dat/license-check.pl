#!/usr/bin/env perl

use strict;
use warnings;

use File::Basename;
use Cwd q(abs_path);
use File::Find;
use List::Util qw(first);
use Text::Glob;

sub indentlevel
{
   my ($spaces) = /^( *)/;
   return int (((length $spaces) - 3) / 2);
}

my %exts = (
   'gfx' => qr/[.]png$/,
   'snd' => qr/[.](?:wav|ogg)$/
);

my %paths = (
   'gfx' => 'ARTWORK_LICENSE',
   'snd' => 'SOUND_LICENSE'
);

my $scriptdir = (fileparse( abs_path($0) ))[1];
chdir $scriptdir or die "Couldn't chdir to $scriptdir";

for my $path (sort keys %paths) {
   my (@tree, @files, @licensed);

   find({ no_chdir => 1, wanted => sub {
      push @files, $_ if -f $_ and $_ =~ /$exts{$path}/ 
   } }, $path);

   my $licensepath = sprintf '%s/%s', $path, $paths{$path};
   open my $fh, '<', $licensepath or die "Couldn't open $licensepath";
   my $last = -1;
   while (<$fh>) {
      next if substr($_, 0, 11) eq ' * Author: ' or not /^( *\* )(.+)$/;

      my ($prefix, $file) = ($1, $2);
      my $indent = indentlevel($prefix);

      if ($indent <= $last) {
         pop @tree foreach 0 .. ($last - $indent);
      }

      push @licensed, glob join('/', $path, @tree, $file);

      push @tree, $file;
      $last = $indent;
   }
   close $fh;

   for my $file (@files) {
      print $file, "\n" if not first { $_ eq $file } @licensed;
   }
}
