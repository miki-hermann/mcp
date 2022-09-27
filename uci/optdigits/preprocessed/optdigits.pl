#!/usr/bin/perl

# produces optdigits.txt

for my $i (0..63) {
    print "id$i\t=\t$i: up [";
    for my $j (0..16) {
	print " $j"
    }
    print "];\n"
}
