#!/usr/bin/perl

if (scalar(@ARGV) < 3 || !($ARGV[0] =~ /-f/)) {
  printf STDERR "Usage: asmpatch.pl -f intercept-function file [file2 file3 ...]\n";
  exit 1;
}

shift ARGV;
$func = shift ARGV;

while ($file = shift ARGV) {

  if (system("cp -f $file $file.orig")) {
    printf "asmpatch: Can't open file '$file'\n";
    next;
  }

  open(ASMIN, "<$file.orig");
  open(ASMOUT, ">$file");

  while (<ASMIN>) {
    if (/^(\s+)call\s+([^\s]+)$/) {
      $s = $1;
      $l = $2;
      if ($l =~ /^\*(.+)$/) {
	printf ASMOUT "%spushl %s\n", $s, $1;
      } else {
	printf ASMOUT "%spushl %s%s\n", $s, '$', $l;
    }
    printf ASMOUT "%scall %s\n", $s, $func;
  } else {
    print ASMOUT;
  }
}
}
