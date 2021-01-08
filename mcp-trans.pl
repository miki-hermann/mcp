#!/usr/bin/perl

###########################################################################
#                                                                         #
#                                                                         #
#	       Multiple Characterization Problem (MCP)                    #
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
#       File:    mcp-trans.pl                                             #
#                                                                         #
#       Copyright (c) 2019 - 2020                                         #
#                                                                         #
# Given a meta-description of a data file, this software generates the    #
# Boolean matrix input for mcp-seq, mcp-mpi, mcp-pthread, and mcp-hybrid. #
#                                                                         #
# This software has been created within the ACCA Project.                 #
#                                                                         #
#                                                                         #
###########################################################################

# use Text::CSV;
use Data::Dumper;

use constant SENTINEL => -1;

use constant {
    FALSE => 0,
    TRUE  => 1
};

use constant {
    LOCAL  => 0,
    GLOBAL => 1
};

my $infile   = 'STDIN';
my $infh     = STDIN;
my $outfile  = 'STDOUT';
my $outfh    = STDOUT;
my $metafile = '';
my $index    = LOCAL;
my $offset   = 0;

################################################################################
#
# parameter input: in-, out-, and metafile identification;
#		   local/global index [default = local]
#		   offset [default = 0]
#
################################################################################

while (scalar @ARGV > 0) {
    if ($ARGV[0] eq '-i'
	|| $ARGV[0] eq '--input') {
	$infile = $ARGV[1];
    } elsif ($ARGV[0] eq '-o'
	     || $ARGV[0] eq '--output') {
	$outfile = $ARGV[1];
    } elsif ($ARGV[0] eq '-m'
	     || $ARGV[0] eq '--meta') {
	$metafile = $ARGV[1];
    } elsif ($ARGV[0] eq '--index') {
	if ($ARGV[1] eq 'l'
	    || $ARGV[1] eq 'loc'
	    || $ARGV[1] eq 'local') {
	    $index = LOCAL;
	} elsif ($ARGV[1] eq 'g'
		 || $ARGV[1] eq 'glob'
		 || $ARGV[1] eq 'global') {
	    $index = GLOBAL;
	} else {
	    die "+++ argument error: " . $ARGV[0] . $ARGV[1] . "\n"
	}
    } elsif ($ARGV[0] eq '--offset') {
	$offset = $ARGV[1];
    } else {
	die "+++ argument error: " . $ARGV[0] . "\n"
    }
    shift @ARGV;
    shift @ARGV;
}
if ($infile ne 'STDIN' && $outfile eq 'STDOUT') {
    $outfile = $infile;
    $outfile =~ s/\.[^\.]*$/.mat/;
}

################################################################################
#
# meta source input
#
################################################################################

open(META, "<", $metafile) or die "+++ cannot open < metafile $metafile: $!\n";

my $msrc = '';			# meta source
my $lineno = 0;			# line number
while (my $line = <META>) {
    $lineno++;
    chomp $line;
    $line =~ s/#.*$//;
    $line =~ s/^\s+//;
    $line =~ s/\s+$//;
    $line =~ s/\s+/ /g;
    my $lngt = length $line;
    if ($lngt > 0) {
	$msrc .= ' #' . $lineno . '# ' . $line;
    }
}

close META;

################################################################################
#
# scanner (lex)
#
################################################################################

use constant {			# tokens
    ERROR  =>  0,		#
    EQUAL  =>  1,		# =
    COLON  =>  2,		# :
    SCOL   =>  3,		# ;
    LBRA   =>  4,		# [
    RBRA   =>  5,		# ]
    MINUS  =>  6,		# -
    QMARK  =>  7,		# ?
    STRING =>  8,		# string [a-zA-Z_]\w*
    NUM    =>  9,		# numeric \d+
    FLOAT  => 10,		# floating point
    IDENT  => 11,		# 'ident'
    BOOL   => 12,		# 'bool'
    ENUM   => 13,		# 'enum'
    UP     => 14,		# 'up'
    DOWN   => 15,		# 'down'
    INT    => 16,		# 'int'
    DJ     => 17,		# 'dj'
    OVER   => 18,		# 'over'
    SPAN   => 19,		# 'span'
    WARP   => 20		# 'warp'
};
my @token_string = ('ERROR', 'EQUAL', 'COLON',  'SCOL', 'LBRA', 'RBRA',
		    'MINUS', 'QMARK', 'STRING', 'NUM',  'FLOAT',
		    'IDENT', 'BOOL',  'ENUM',   'UP',   'DOWN', 'INT',
		    'DJ',    'OVER',  'SPAN',   'WARP');

my $yytext;
my $anything = FALSE;
sub yylex {
    $msrc =~ s/^\s+//;
    if ($msrc =~ s/^#(\d+)#//) {
	$lineno = $1;
	$msrc =~ s/^\s+//;
    }
    $yytext = '';
    if ($anything == TRUE && $msrc =~ s/^\]//) {
    	$anything = FALSE;
    	return RBRA
    } elsif ($anything == TRUE && $msrc =~ s/^://) {
	return COLON
    } elsif ($anything == TRUE && $msrc =~ s/^(\S+)\]/\]/) {
    	$yytext = $1;
    	return STRING
    } elsif ($anything == TRUE && $msrc =~ s/^(\S+) / /) {
    	$yytext = $1;
    	return STRING
    } elsif ($msrc =~ s/^=//) {
	return EQUAL
    } elsif ($msrc =~ s/^://) {
	return COLON
    } elsif ($msrc =~ s/^;//) {
	$anything = FALSE;
	return SCOL
    } elsif ($msrc =~ s/^\[//) {
	$anything = TRUE;
	return LBRA
    } elsif ($msrc =~ s/^\]//) {
	$anything = FALSE;
	return RBRA
    } elsif ($msrc =~ s/^-//) {
	return MINUS
    } elsif ($msrc =~ s/^ident(\W)/$1/) {
	return IDENT
    } elsif ($msrc =~ s/^bool(\W)/$1/) {
	return BOOL
    } elsif ($msrc =~ s/^enum(\W)/$1/) {
	return ENUM
    } elsif ($msrc =~ s/^up(\W)/$1/) {
	return UP
    } elsif ($msrc =~ s/^down(\W)/$1/) {
	return DOWN
    } elsif ($msrc =~ s/^int(\W)/$1/) {
	return INT
    } elsif ($msrc =~ s/^dj(\W)/$1/) {
	return DJ
    } elsif ($msrc =~ s/^over(\W)/$1/) {
	return OVER
    } elsif ($msrc =~ s/^span(\W)/$1/) {
	return SPAN
    } elsif ($msrc =~ s/^warp(\W)/$1/) {
	return WARP
    } elsif ($msrc =~ s/^\?//) {
	return QMARK
    } elsif ($msrc =~ s/^(\d*\.\d+)//) {
	$yytext = $1;
	return FLOAT
    } elsif ($msrc =~ s/^(\d+)//) {
	$yytext = $1;
	return NUM
    } elsif ($msrc =~ s/^([^ #=:;-\?\[\]]+)//) {
	$yytext = $1;
	return STRING
    }
}

################################################################################
#
# error handling and recovery
#
################################################################################

my $errorflag = FALSE;

sub error {
    $errorflag = TRUE;
    my $message = shift @_;
    warn "+++ error on line $lineno: $message\n";
}

sub flush {
    my $string = shift @_;
    my $include = shift @_;
    $msrc =~ s/^[^$string]*//;
    if ($include) {
	$msrc =~ s/^$string//;
    }
}

################################################################################
#
# global variables
#
################################################################################
my %symtab;		# symbol table for descriptions
my $desc;		# identifier of the specification
my @description = ('');	# array of string: identifiers of the specifications
my $orig_column;	# column in the original file
my $target_item;	# which item is it in the target file
my $ident;		# column for the ident
my @type;		# type of the item
my @target;		# which original column goes to this target
my @args;		# arguments of a specification; array of array
my $qmarkcount = 0;	# number of lines with question mark = missing data
my $linecount  = 0;	# number of produced lines
################################################################################

################################################################################
#
# syntactic analysis, creation of internal structures
#
################################################################################

sub specification {
    my $token;
    my $minus;
    my $spec = yylex();
    if ($spec == IDENT) {
	$ident = $orig_column;
	$description[0] = $desc;
	return;
    } else {
	push @description, $desc;
    }

    if ($spec >= BOOL && $spec <= WARP) {
	$type[$orig_column] = $spec;
	$target[++$target_item] = $orig_column;
    } else {
	error 'wrong start of specification';
	flush($token_string[SCOL], TRUE);
	return;
    }

    if ($spec >= BOOL && $spec <= DOWN) {
	$token = yylex();
	if ($token != LBRA) {
	    error 'missing [';
	}
    }

    my $number_of_arguments = 0;
    if ($spec >= BOOL && $spec <= DOWN) {
	while (TRUE) {
	    $token = yylex();
	    if ($token == RBRA) {
		last;
	    } elsif ($token == SCOL) {
		error 'missing ]';
		$msrc = ';' . $msrc;
		last;
	    } elsif (length $msrc == 0) {
		error '+++ unexpected EOF +++';
		die "\n";
	    }
	    $minus = 1;
	    if ($token == MINUS) {
		$minus = -1;
		$token = yylex();
	    }
	    if ($token >= NUM && $token <= FLOAT) {
		push @{$args[$target_item]}, $minus * $yytext;
	    } elsif ($minus == -1) {
		error 'after - must follow num or float';
		flush($token_string[SCOL], FALSE);
		last;
	    } elsif ($token == STRING) {
		push @{$args[$target_item]}, $yytext;
	    } elsif ($token == QMARK) {
		warn "+++ question mark on line $lineno ignored\n";
	    }
	    $number_of_arguments++;
	}
	if ($spec == BOOL && $number_of_arguments != 2) {
	    error 'bool must have 2 arguments';
	} elsif ($number_of_arguments == 0) {
	    error 'no arguments?';
	}
    } elsif ($spec == INT) {
	for my $i (0..1) {
	    $token = yylex();
	    $minus = 1;
	    if ($token == MINUS) {
		$minus = -1;
		$token = yylex();
	    }
	    if ($token == NUM) {
		$args[$target_item][$i] = $minus * $yytext;
	    } else {
		error "$yytext must be an integer";
		flush($token_string[SCOL], FALSE);
		last;
	    }
	}
	if ($args[$target_item][0] > $args[$target_item][1]) {
	    error 'first argument must be smaller or equal than the second';
	}
    } elsif ($spec >= DJ && $spec <= OVER) {
	$token = yylex();
	if ($token == NUM) {
	    $args[$target_item][0] = $yytext;
	} else {
	    error 'ćardinality must be a positive integer';
	    flush($token_string[SCOL], TRUE);
	    return;
	}
	$token = yylex();
	$minus = 1;
	if ($token == MINUS) {
	    $minus = -1;
	    $token = yylex();
	}
	if ($token == NUM || $token == FLOAT) {
	    $args[$target_item][1] = $minus * $yytext;
	} else {
	    error 'minimum must be a number';
	    flush($token_string[SCOL], TRUE);
	    return;
	}
	$token = yylex();
	$minus = 1;
	if ($token == MINUS) {
	    $minus = -1;
	    $token = yylex();
	}
	if ($token == NUM || $token == FLOAT) {
	    $args[$target_item][2] = $minus * $yytext;
	} else {
	    error 'maximum must be a number';
	    flush($token_string[SCOL], TRUE);
	    return;
	}
	if ($spec == OVER) {
	    $token = yylex();
	    if ($token == NUM || $token == FLOAT) {
		$args[$target_item][3] = $yytext;
	    } else {
		error 'overlap must be a positive number';
		flush($token_string[SCOL], TRUE);
		return;
	    }
	}
    } elsif ($spec >= SPAN && $spec <= WARP) {
	$token = yylex();
	if ($token == NUM || $token == FLOAT) {
	    $args[$target_item][0] = $yytext;
	} else {
	    error 'length must be a positive number';
	    flush($token_string[SCOL], TRUE);
	    return;
	}
	$token = yylex();
	$minus = 1;
	if ($token == MINUS) {
	    $minus = -1;
	    $token = yylex();
	}
	if ($token == NUM || $token == FLOAT) {
	    $args[$target_item][1] = $minus * $yytext;
	} else {
	    error 'minimum must be a number';
	    flush($token_string[SCOL], TRUE);
	    return;
	}
	$token = yylex();
	$minus = 1;
	if ($token == MINUS) {
	    $minus = -1;
	    $token = yylex();
	}
	if ($token == NUM || $token == FLOAT) {
	    $args[$target_item][2] = $minus * $yytext;
	} else {
	    error 'maximum must be a number';
	    flush($token_string[SCOL], TRUE);
	    return;
	}
	if ($spec == WARP) {
	    $token = yylex();
	    if ($token == NUM || $token == FLOAT) {
		$args[$target_item][3] = $yytext;
	    } else {
		error 'overlap must be a positive number';
		flush($token_string[SCOL], TRUE);
		return;
	    }
	}
    } else {
	die "+++ I should not be here +++\n"
    }
}

sub command_line {
    my $token = yylex();
    if ($token == STRING) {
	$desc = $yytext;
    } else {
	error 'description must start with a string';
	flush($token_string[EQUAL]
	      . $token_string[COLON]
	      . $token_string[SCOL],
	      FALSE);
    }
    if (exists $symtab{$desc}) {
	error $desc . ' already exists';
    } else {
	$symtab{$desc} = TRUE;
    }

    $token = yylex();
    if ($token != EQUAL) {
	error 'missing =';
	flush($token_string[NUM], FALSE);
    }

    $token = yylex();
    if ($token == NUM) {
	$orig_column = $yytext;
    } else {
	error 'column number missing';
	flush($token_string[COLON], FALSE);
    }

    $token = yylex();
    if ($token != COLON) {
	error 'missing :';
	flush($token_string[SCOL], TRUE);
	return;
    }

    specification ();

    $token = yylex();
    if ($token != SCOL) {
	error 'missing ; on previous line?';
	flush($token_string[SCOL], TRUE);
    }
}

sub program {
    $target_item = 0;
    while (length $msrc > 0) {
	command_line();
    }
    if(! defined $ident) {
	error 'no identifier';
    }
}

program();
if ($errorflag) {
    die "+++ errors in meta file $metafile +++\n"
}

warn "+++ meta file $metafile OK, start processing input file $infile\n";

################################################################################
#
# code generation
#
################################################################################

# $Data::Dumper::Indent = 3;
# print Dumper(@type), "\n\n";
# print Dumper(@target), "\n\n";
# print Dumper(@args), "\n\n";

if ($infile ne 'STDIN') {
    open ($infh,  "<", $infile)  or die "+++ Cannot open < $infile: $!\n";
}
if ($outfile ne 'STDOUT') {
    open ($outfh, ">", $outfile) or die "+++ Cannot open > $outfile: $!\n";
}

sub position {		# position of the first argument in the rest of array
    my @list = @_;
    my $item = $list[0];

    my $i = $#list;
    while ($list[$i] ne $item) {$i--}
    return $i-1;
}

################################################################################
#
# header
#
################################################################################

print $outfh "1 0\n";
# print $outfh $description[0], ' ';
my $item_length;
my $varnum = 0;
for my $tgt (1..$#target) {
    if ($index == LOCAL) {
	$varnum = 0;
    }
    my $ocl = $target[$tgt];
    if ($type[$ocl] == BOOL) {
	$item_length = 1;
    } elsif ($type[$ocl] >= ENUM && $type[$ocl] <= DOWN) {
	$item_length = scalar @{$args[$tgt]};
    } elsif ($type[$ocl] == INT) {
	$item_length = $args[$tgt][1] - $args[$tgt][0] + 1;
    } elsif ($type[$ocl] >= DJ && $type[$ocl] <= OVER) {
	$item_length = $args[$tgt][0];
    } elsif ($type[$ocl] >= SPAN && $type[$ocl] <= WARP) {
	my $ratio = ($args[$tgt][2] - $args[$tgt][1]) / $args[$tgt][0];
	$item_length = int $ratio;
	$item_length += ($ratio - $item_length > 0) ? 1 : 0;
    }
    for my $i (1..$item_length) {
	print $outfh
	    $description[$tgt], '_',
	    ($type[$ocl] >= BOOL && $type[$ocl] <= INT
	     ? $item_length - $i
	     : $i - 1) + $varnum + $offset;

	# positive case
	print ':';
	if ($type[$ocl] == BOOL) {
	    # print $description[$tgt], '==', $args[$tgt][1];
	    print $description[$tgt], '==', $args[$tgt][0];
	} elsif ($type[$ocl] == ENUM) {
	    print $description[$tgt], '==', $args[$tgt][$item_length - $i];
	} elsif ($type[$ocl] == UP) {
	    print $description[$tgt], '>=', $args[$tgt][$item_length - $i];
	} elsif ($type[$ocl] == DOWN) {
	    print $description[$tgt], '<=', $args[$tgt][$item_length - $i];
	} elsif ($type[$ocl] == INT) {
	    print $description[$tgt], '==', $args[$tgt][1] - $i + 1;
	} elsif ($type[$ocl] >= DJ && $type[$ocl] <= WARP) {
	    my $min = $args[$tgt][1];
	    my $max = $args[$tgt][2];
	    my $ilngt = $type[$ocl] <= OVER
		? ($max - $min) / $args[$tgt][0]
		: $args[$tgt][0];
	    my $over = $type[$ocl] == OVER || $type[$ocl] == WARP
		? $args[$tgt][3]
		: 0.0;
	    print $min + $lngt * ($i-1) - $over/2,
		'<=',
		$description[$tgt],
		'<',
		$min + $ilngt * $i + $over/2;
	}

	# negative case
	print ':';
	if ($type[$ocl] == BOOL) {
	    # print $description[$tgt], '==', $args[$tgt][0];
	    print $description[$tgt], '==', $args[$tgt][1];
	} elsif ($type[$ocl] == ENUM) {
	    print $description[$tgt], '!=', $args[$tgt][$item_length - $i];
	} elsif ($type[$ocl] == UP) {
	    print $description[$tgt], '<', $args[$tgt][$item_length - $i];
	} elsif ($type[$ocl] == DOWN) {
	    print $description[$tgt], '>', $args[$tgt][$item_length - $i];
	} elsif ($type[$ocl] == INT) {
	    print $description[$tgt], '!=', $args[$tgt][1] - $i + 1;
	} elsif ($type[$ocl] >= DJ && $type[$ocl] <= WARP) {
	    my $min = $args[$tgt][1];
	    my $max = $args[$tgt][2];
	    my $ilngt = $type[$ocl] <= OVER
		? ($max - $min) / $args[$tgt][0]
		: $args[$tgt][0];
	    my $over = $type[$ocl] == OVER || $type[$ocl] == WARP
		? $args[$tgt][3]
		: 0.0;
	    print $description[$tgt],
		'<', $min + $lngt * ($i-1) - $over/2,
		'||',
		$description[$tgt],
		'>=',
		$min + $ilngt * $i + $over/2;
	}

	print ' ';
    }
    $varnum += $item_length;
}
print $outfh "\n";

################################################################################
#
# boolean matrix
#
################################################################################

$lineno = 0;
while (my $line1 = <$infh>) {
    $lineno++;
    chomp $line1;
    $line1 =~ s/\r//;
    $line1 =~ s/^\s+//;
    $line1 =~ s/\s+$//;
    if (length $line1 == 0) {
	next
    }

    my $line = '';
    my $is_string = FALSE;
    my @line1 = split //, $line1;
    for my $char (@line1) {
	if ($char eq '"') {
	    $is_string = TRUE - $is_string;
	    $line .= ' ';
	} elsif ($is_string && $char eq ' ') {
	    $line .= '_';
	} elsif ($is_string && $char eq '?') {
	    $line .= '<>';
	} elsif ($is_string && ($char eq ',' || $char eq ';')) {
	    $line .= '.';
	} else {
	    $line .= $char;
	}
    }

    if ($line =~ /\?/) {
	$qmarkcount++;
	next;
    } else {
	$linecount++;
    }

    # $line =~ s/"//g;
    $line =~ s/[,;]/ /g;
    my @line = split / +/, $line;
    
    print $outfh $line[$ident];
    my $position;
    for my $tgt (1..$#target) {
	my $ocl = $target[$tgt];
	if ($type[$ocl] == BOOL) {
	    $position = position $line[$ocl], @{$args[$tgt]};
	    if ($position == SENTINEL) {
		error $line[$ocl] .
		    " not in bool specification on coordinate $ocl";
	    } else {
		print $outfh ' ', $position;
	    }
	} elsif ($type[$ocl] == ENUM) {
	    my @eud_args = @{$args[$tgt]};
	    $position = position $line[$ocl], @eud_args;
	    if ($position == SENTINEL) {
		error $line[$ocl],  .
		    " not in enum specification on coordinate $ocl";
	    } else {
		for my $j (0..$#eud_args) {
		    print $outfh ($#eud_args - $j == $position ? ' 1' : ' 0');
		}
	    }
	} elsif ($type[$ocl] == UP) {
	    my @eud_args = @{$args[$tgt]};
	    $position = position $line[$ocl], @eud_args;
	    if ($position == SENTINEL) {
		error $line[$ocl] .
		    " not in up specification on coordinate $ocl";
	    } else {
		for my $j ($position+1..$#eud_args) {
		    print $outfh ' 0';
		}
		for my $j (0..$position) {
		    print $outfh ' 1';
		}
	    }
	} elsif ($type[$ocl] == DOWN) {
	    my @eud_args = @{$args[$tgt]};
	    $position = position $line[$ocl], @eud_args;
	    if ($position == SENTINEL) {
		error $line[$ocl] .
		    " not in down specification on coordinate $ocl";
	    } else {
		for my $j (0..$position-1) {
		    print $outfh ' 0';
		}
		for my $j ($position..$#eud_args) {
		    print $outfh ' 1';
		}
	    }
	} elsif ($type[$ocl] == INT) {
	    my $imin = $args[$tgt][0];
	    my $imax = $args[$tgt][1];
	    if ($line[$ocl] =~ /^[^-?\d+]$/) {
		error $line[$ocl] .
		    " not an integer on coordinate $ocl";
	    } elsif ($line[$ocl] < $imin || $line[$ocl] > $imax) {
		error $line[$ocl] .
		    " out of bounds $imin..$imax on coordinate $ocl";
	    } else {
		for (my $j = $imax; $j >= $imin; --$j) {
		    print $outfh ($j == $line[$ocl] ? ' 1' : ' 0');
		}
	    }
	} elsif ($type[$ocl] >= DJ && $type[$ocl] <= WARP) {
	    my $min = $args[$tgt][1];
	    my $max = $args[$tgt][2];
	    my $icard;
	    my $ilngt;
	    my $over = 0;
	    if ($type[$ocl] <= OVER) {
		$icard = $args[$tgt][0];
		$ilngt = ($max - $min) / $icard;
	    } elsif ($type[$ocl] >= SPAN) {
		$ilngt = $args[$tgt][0];
		my $ratio = ($max - $min) / $ilngt;
		$icard = int $ratio;
		$icard += ($ratio - $icard > 0) ? 1 : 0;
	    }
	    if ($type[$ocl] == OVER || $type[$ocl] == WARP) {
		$over = $args[$tgt][3];
	    }
	    if ($line[$ocl] =~ /^[^-?\d+]$/
		|| $line[$ocl] =~ /^[^-?\d*\.\d+]$/) {
		error $line[$ocl] .
		    " is not a number on coordinate $ocl";
	    } elsif ($line[$ocl] < $min - $over / 2
		     || $line[$ocl] >= $max + $over / 2) {
		error $line[$ocl] .
		    " out of bounds " .
		    ($min - $over/2) .
		    ".." .
		    ($max + $over/2) .
		    " on coordinate $ocl";
	    } else {
		for my $j (1..$icard) {
		    print $outfh
			($line[$ocl] >= $min + $ilngt * ($j - 1) - $over / 2
			 &&
			 $line[$ocl] <  $min + $ilngt * $j + $over / 2
			 ? ' 1' : ' 0');
		}
	    }
	} else {
	    die "+++ you should not be here +++\n";
	}
    }
    print $outfh "\n";
}

if ($qmarkcount > 0) {
    warn "+++ $qmarkcount lines skipped due to missing values represented by '?'\n";
}
warn "+++ produced $linecount", ($linecount == 1 ? ' line' : ' lines'), "\n";

close $infh;
close $outfh;

if ($errorflag) {
    if ($outfile ne STDOUT) {
	unlink $outfile;
	warn "+++ output file $outfile deleted\n";
    }
    warn "+++ runtime errors in data file $infile\n";
} else {
    if ($outfile ne STDOUT) {
	warn "+++ output file $outfile generated\n";
    }
    warn "+++ transformation successful\n";
}


################################################################################
#
# EOF
#
################################################################################
