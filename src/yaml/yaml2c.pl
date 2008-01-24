#!/usr/bin/perl

# yaml2c.pl - take a YAML file and create a C file.

sub munge_name($)
{
    my $name = shift;
    $name =~ tr!,/. !____!;
    return $name;
}

open my $IN, $ARGV[0] or die "Couldn't open file!";
my $name = munge_name($ARGV[1]);
print "char $name [] = {\n";
my $count = 0;
my $ch;
while() {
    $ch = getc($IN);
    last if !defined($ch);
    printf ' 0x%02x,', ord($ch);
    $count += 6;
    if($count >= 78) { 
	print "\n";
	$count = 0;
    }
}
print "\n};\n"
