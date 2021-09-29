#!/usr/bin/perl

my @shape = qw(round square octagon);
my @smiling = qw(yes no);
my @holding = qw(sword balloon flag);
my @jcolor = qw(red yellow green blue);
my @tie = qw(yes no);

while (my $line = <>) {
    chomp $line;
    $line =~ s/^ +//;
    my @line = split / /, $line;
    my @output;
    push @output, ($line[0],
		   $shape[$line[1]-1],
		   $shape[$line[2]-1],
		   $smiling[$line[3]-1],
		   $holding[$line[4]-1],
		   $jcolor[$line[5]-1],
		   $tie[$line[6]-1]);
    my $output = join ' ', @output;
    print $output, "\n";
}
