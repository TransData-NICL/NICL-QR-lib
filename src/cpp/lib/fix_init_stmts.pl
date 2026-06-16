#!/usr/bin/perl
use strict;
use warnings;

my $file = $ARGV[0] or die "Usage: $0 <file>\n";
open my $fh, '<', $file or die "Can't open $file: $!\n";
my @lines = <$fh>;
close $fh;

my @result;
my @pending_closes; # array of [indent, brace_depth]

for my $i (0 .. $#lines) {
    my $line = $lines[$i];
    
    # Match init-statement: if (DECL; COND) {
    if ($line =~ /^(\s*)if \(((?:const )?auto\s+\w+\s*=\s*[^;]+);\s*(.+)\)\s*\{\s*$/) {
        my ($indent, $decl, $cond) = ($1, $2, $3);
        push @result, "$indent\{\n";
        push @result, "$indent    $decl;\n";
        push @result, "$indent    if ($cond) \{\n";
        # Track: we need to close when the if's { closes
        # Start depth at 1 (we just opened a {)
        push @pending_closes, [$indent, 1];
        next;
    }
    
    # Update brace tracking
    if (@pending_closes) {
        my $opens = () = $line =~ /\{/g;
        my $closes_count = () = $line =~ /\}/g;
        
        # Update all tracked entries
        for my $entry (@pending_closes) {
            $entry->[1] += $opens - $closes_count;
        }
        
        push @result, $line;
        
        # Check if any tracked blocks have closed (depth <= 0)
        my @new_pending;
        for my $entry (@pending_closes) {
            if ($entry->[1] <= 0) {
                push @result, "$entry->[0]\}\n";
            } else {
                push @new_pending, $entry;
            }
        }
        @pending_closes = @new_pending;
        next;
    }
    
    push @result, $line;
}

open my $out, '>', $file or die "Can't write $file: $!\n";
print $out @result;
close $out;
print "Fixed init-statements in $file\n";
