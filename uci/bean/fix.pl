#!/usr/bin/perl

while (my $line = <>) {
    chomp $line;
    $line =~ s/([0-9]*)\.([0-9]+)E-4/0.000\1\2/;
    print $line, "\n";
}
