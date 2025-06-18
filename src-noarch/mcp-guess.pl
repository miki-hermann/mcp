#!/usr/bin/perl

###########################################################################
#                                                                         #
#                                                                         #
#	       Multiple Classification   Problem (MCP)                    #
#                                                                         #
#	Author:   Miki Hermann                                            #
#	e-mail:   hermann@lix.polytechnique.fr                            #
#	Address:  LIX (CNRS UMR 7161), Ecole Polytechnique, France        #
#                                                                         #
#	Author: Gernot Salzer                                             #
#	e-mail: gernot.salzer@tuwien.ac.at                                #
#	Address: Technische Universitaet Wien, Vienna, Austria            #
#                                                                         #
#	Version: all                                                      #
#       File:    mcp-guess.pl                                             #
#                                                                         #
#      Copyright (c) 2019 - 2025                                          #
#                                                                         #
# Guess a skeleton of a meta file from a (CSV) data file                  #
#                                                                         #
###########################################################################


use List::Util qw(min max);
use List::MoreUtils qw(uniq);

use constant {
    FALSE => 0,
    TRUE  => 1
};

use constant  {
    ENUM_RATIO => 0.1,
    ENUM_MAX => 200
};

use constant QMARK => '?';

use constant {
    UNDEF  => 0,
    INT    => 1,
    FLOAT  => 2,
    STRING => 3
};
my @name = ('', 'int', 'float', 'string');

my $errorflag  = FALSE;	# error indicator in file
my $qflag      = FALSE;	# qustion mark flag
my @data;		# array of array data[col][row]
my @type;		# column type
my $row_count  = 0;	# number of input rows
my $row_length = 0;	# number of items in row
my @flength;		# length of decimal part

my $infile   = 'STDIN';
my $infh     = STDIN;
my $outfile  = 'STDOUT';
my $outfh    = STDOUT;

while (scalar @ARGV > 0) {
    if ($ARGV[0] eq '-i'
	|| $ARGV[0] eq '--input') {
	$infile = $ARGV[1];
    } elsif ($ARGV[0] eq '-o'
	     || $ARGV[0] eq '--output') {
	$outfile = $ARGV[1];
    } else {
	die "+++ argument error: " . $ARGV[0] . "\n"
    }
    shift @ARGV;
    shift @ARGV;
}
if ($infile ne 'STDIN' && $outfile eq 'STDOUT') {
    $outfile = $infile;
    $outfile =~ s/\.[^\.]*$/.txt/;
}

if ($infile ne 'STDIN') {
    open ($infh,  "<", $infile)  or die "+++ Cannot open < $infile: $!\n";
}
if ($outfile ne 'STDOUT') {
    open ($outfh, ">", $outfile) or die "+++ Cannot open > $outfile: $!\n";
}

while (my $line = <$infh>) {
    $row_count++;
    chomp $line;
    $line =~ s/\r//;
    $line =~ s/^\s+//;
    $line =~ s/\s+$//;
    if (length $line == 0) {
	next
    }

    # $line =~ s/"//g;
    my $in_string = FALSE;
    my @line = split //, $line;
    for my $i (0..$#line) {
	if ($line[$i] eq '"') {
	    $in_string = TRUE - $in_string;
	    $line[$i] = ' ';
	} elsif ($in_string && $line[$i] =~ /^\s+$/) {
	    $line[$i] = '_';
	} elsif ($in_string && $line[$i] =~ /^\?$/) {
	    $line[$i] = '<>';
	} elsif ($in_string && $line[$i] =~ /^[,;]$/) {
	    $line[$i] = '.';
	}
    }
    $line = join '', @line;
    $line =~ s/[,;]/ /g;
    @line = split / +/, $line;
    for my $col (0..$#line) {
	push @{$data[$col]}, $line[$col]
    }

    my $rs = scalar @line;
    if ($row_length == 0) {
	$row_length = $rs;
    } elsif ($row_length != $rs) {
	$errorflag = TRUE;
	print $outfh "+++ item count discrepancy on line $row_count\n";
	print $outfh "+++ row length = $row_length, row size = $rs\n";
    }
}

if ($errorflag == TRUE) {
    print $outfh "+++ errors in data file\n";
    exit 1;
}

for my $col (0..$#data) {
    $flength[$col] = 0;
    my @row = @{$data[$col]};
    for my $i (0..$#row) {
	# print $outfh "*** testing '", $row[$i], "'\n";
	if ($type[$col] == INT) {
	    if ($row[$i] =~ /^-?\d*\.(\d+)$/
		|| $row[$i] =~ /^-?\d+\.(\d+)e(\-|\+)\d+$/) {
		$flength[$col] = max($flength[$col], length $1);
		$type[$col] = FLOAT
	    } elsif ($row[$i] ne QMARK
		     &&
		     !($row[$i] =~ /^-?\d+$/)) {
		$type[$col] = STRING
	    } elsif ($row[$i] eq QMARK) {
		$qflag = TRUE;
	    }
	} elsif ($type[$col] == FLOAT) {
	    if ($row[$i] ne QMARK
		&& !($row[$i] =~ /^-?\d*\.\d+$/)
		&& !($row[$i] =~ /^-?\d+\.(\d+)e(\-|\+)\d+$/)
		&& !($row[$i] =~ /^-?\d+$/)) {
		$type[$col] = STRING
	    } elsif ($row[$i] eq QMARK) {
		$qflag = TRUE;
	    }
	} elsif (!defined($type[$col])) {
	    if ($row[$i] =~ /^-?\d+$/) {
		$type[$col] = INT
	    } elsif ($row[$i] =~ /^-?\d*\.(\d+)$/
		     || $row[$i] =~ /^-?\d+\.(\d+)e(\-|\+)\d+$/) {
		$flength[$col] = max($flength[$col], length $1);
		$type[$col] = FLOAT
	    } elsif ($row[$i] ne QMARK) {
		$type[$col] = STRING
	    } elsif ($row[$i] eq QMARK) {
		$qflag = TRUE;
	    }
	}
	# print $outfh "*** conclusion = ", $name[$type[$col]], "\n\n";
    }
}

my $dfmt = '= %' . (length $#data) . 'd: ';
my $sfmt = 'id%-' .  (length $#data) . 's ';
for my $col (0..$#data) {
    printf $outfh $sfmt, $col;
    printf $outfh $dfmt, $col;
    my @row = grep {$_ ne QMARK} @{$data[$col]};
    if ($type[$col] == INT
	|| $type[$col] == FLOAT) {
	@row = uniq (sort {$a <=> $b} @row);
    } elsif ($type[$col] == STRING) {
	@row = uniq (sort {$a cmp $b} @row);
    }
    my $is_enum = (scalar @row) <= ENUM_MAX
	||
	(scalar @row) <= $row_count * ENUM_RATIO;
    if (scalar @row == 2) {
	print $outfh "bool "
    } elsif ($is_enum && $type[$col] != FLOAT) {
	print $outfh "enum ";
	print $outfh $name[$type[$col]], ' ';
    } else {
	print $outfh $name[$type[$col]], ' ';
    }
    if ($is_enum && $type[$col] != FLOAT) {
	print $outfh '[', (join ' ', @row), ']';
    } elsif ($type[$col] == FLOAT) {
	my $format = '%.' . $flength[$col] . 'f';
	printf $outfh $format . ' ' . $format, min(@row), max(@row);
    } else {
	print $outfh min(@row), ' ', max(@row);
    }
    print $outfh ";\t# card ", (scalar @row);
    my $is_cons = ($type[$col] == INT);
    if ($is_cons == TRUE) {
	for my $i (1..$#row) {
	    if ($row[$i] != $row[$i-1]+1) {
		$is_cons = FALSE;
		last;
	    }
	}
    }
    if ($is_cons) {
	print $outfh " consecutive";
    }
    print $outfh "\n"
}
if ($qflag == TRUE) {
    print $outfh  "# missing values represented by '?' encountered\n";
}

close $infh;
close $outfh;

################################################################################
#
# EOF
#
################################################################################
