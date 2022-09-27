#!/usr/bin/perl

# optdigits-x.orig -> optdigits-x.mat

my $basename;
$ARGV[0] =~ /([a-z-]*)\.orig/;
$basename = $1;

open (my $infh, "<", $ARGV[0])
    or die "+++ cannot open input file $ARGV[0]: $!\n";
open (my $outfh, ">", $basename.".mat")
    or die "+++ cannot open input file ", $basename . ".mat", ": $!\n";

print $outfh "0 0\n";
while (my $line = <$infh>) {
    chomp $line;
    for my $i (1..31) {
	my $newline = <$infh>;
	chomp $newline;
	$line .= $newline;
    }
    my @line = split //, $line;
    $line = join ' ', @line;
    my $group = <$infh>;
    chomp $group;
    $group =~ s/^ +//;
    print $outfh $group, ' ', $line, "\n";
}
close $infh;
close $outfh;
print "+++ file $basename.mat created\n";
