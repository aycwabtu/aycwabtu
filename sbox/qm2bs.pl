#
# echo "'--10----', '^^------'" | perl qm2bs.pl
# out[0] = BS_OR(BS_AND(BS_NOT(b_4),b_5),BS_XOR(b_6,b_7));
#
# echo "'--10~~~-'" | perl qm2bs.pl
# out[0] = BS_AND(BS_NOT(BS_XOR(BS_XOR(b_1,b_2),b_3)),BS_AND(BS_NOT(b_4),b_5));
#
#
#
#
#


my $outbit=0;
my $debug=0;

while(<>)
{
    my $line = '';
    my @outterms = undef;
    my $outterm = 0;
    my @minterms = split(',', $_);
    foreach (@minterms)
    {
        m/.*'(.*)'.*/;
        my $term = $1;
        print ">>> $term <<< " if $debug;
        my @vars = undef;
        for (my $i=7; $i >=0; $i--)
        {
            $vars[$i] = substr($term, 7-$i, 1 );
            if ($vars[$i] eq '-') {$vars[$i]='';}
        }
        # 1 AND 1 pairs
        for (my $i=0; $i<7; $i++)
        {
            for (my $j=$i+1; $j<8; $j++)
            {
                if (($vars[$i] eq '1') and ($vars[$j] eq '1'))
                {
                    $vars[$i]="BS_AND(b_$i,b_$j)";
                    $vars[$j]='';
                    print "found 1 in $i $j\n" if $debug;
                }
            }
        }
        # 0 AND 0 pairs
        for (my $i=0; $i<7; $i++)
        {
            for (my $j=$i+1; $j<8; $j++)
            {
                if (($vars[$i] eq '0') and ($vars[$j] eq '0'))
                {
                    $vars[$i]="BS_NOT(BS_OR(b_$i,b_$j))";
                    $vars[$j]='';
                    print "found 0 in $i $j\n" if $debug;
                }
            }
        }
        # 1 AND 0  or 0 AND 1
        for (my $i=0; $i<7; $i++)
        {
            for (my $j=$i+1; $j<8; $j++)
            {
                if (($vars[$i] eq '0') and ($vars[$j] eq '1'))
                {
                    $vars[$i]="BS_AND(BS_NOT(b_$i),b_$j)";
                    $vars[$j]='';
                    print "found 01 in $i $j\n" if $debug;
                }
                if (($vars[$i] eq '1') and ($vars[$j] eq '0'))
                {
                    $vars[$i]="BS_AND(b_$i,BS_NOT(b_$j))";
                    $vars[$j]='';
                    print "found 10 in $i $j\n" if $debug;
                }
            }
        }

        
        # ^ AND ^ pairs
        for (my $i=0; $i<7; $i++)
        {
            for (my $j=$i+1; $j<8; $j++)
            {
                if (($vars[$i] eq '^') and ($vars[$j] eq '^'))
                {
                    $vars[$i]="BS_XOR(b_$i,b_$j)";
                    $vars[$j]='';
                    print "found \^ in $i $j\n" if $debug;
                }
            }
        }

        # BS_XOR pairs
        for (my $i=0; $i<7; $i++)
        {
            for (my $j=$i+1; $j<8; $j++)
            {
                if (($vars[$i] =~ /^BS_XOR/) and ($vars[$j] =~ /^BS_XOR/))
                {
                    $vars[$i]="BS_XOR($vars[$i],$vars[$j])";
                    $vars[$j]='';
                    print "found XOR in $i $j\n" if $debug;
                }
            }
        }
        # remaining ^
        for (my $i=0; $i<7; $i++)
        {
            for (my $j=$i+1; $j<8; $j++)
            {
                if (($vars[$i] =~ /^BS_XOR/) and ($vars[$j] eq '^'))
                {
                    $vars[$i]="BS_XOR($vars[$i],b_$j)";
                    $vars[$j]='';
                    print "found XOR/^ in $i $j\n" if $debug;
                }
            }
        }
        

        # for XNOR negate only once!
        # either '^' or '~' are in the string!
        my $xnor =0;
        # ~ AND ~ pairs
        for (my $i=0; $i<7; $i++)
        {
            for (my $j=$i+1; $j<8; $j++)
            {
                if (($vars[$i] eq '~') and ($vars[$j] eq '~'))
                {
                    $vars[$i]="BS_XOR(b_$i,b_$j)";
                    $vars[$j]='';
                    print "found ~ in $i $j\n" if $debug;
                    $xnor=1;
                }
            }
        }

        # BS_X(N)OR pairs
        for (my $i=0; $i<7; $i++)
        {
            for (my $j=$i+1; $j<8; $j++)
            {
                if (($vars[$i] =~ /^BS_XOR/) and ($vars[$j] =~ /^BS_XOR/) and ($xnor eq 1))
                {
                    $vars[$i]="BS_XOR($vars[$i],$vars[$j])";
                    $vars[$j]='';
                    print "found XNOR in $i $j\n" if $debug;
                }
            }
        }
        # remaining ~
        for (my $i=0; $i<7; $i++)
        {
            for (my $j=$i+1; $j<8; $j++)
            {
                if (($vars[$i] =~ /^BS_XOR/) and ($vars[$j] eq '~') and ($xnor eq 1))
                {
                    $vars[$i]="BS_XOR($vars[$i],b_$j)";
                    $vars[$j]='';
                    print "found XNOR/^ in $i $j\n" if $debug;
                }
            }
        }
        # negate XNOR
        for (my $i=0; $i<7; $i++)
        {
            if (($vars[$i] =~ /^BS_XOR/) and ($xnor eq 1))
            {
                $vars[$i]="BS_NOT($vars[$i])";
                print "negate XOR in $i\n" if $debug;
            }
        }
        

        # only one '1' or '0' is left
        for (my $i=0; $i<7; $i++)
        {
            for (my $j=$i+1; $j<8; $j++)
            {
                if (not ($vars[$i] eq '') and ($vars[$j] eq '1'))
                {
                    $vars[$i]="BS_AND($vars[$i],b_$j)";
                    $vars[$j]='';
                }
                if (not ($vars[$i] eq '') and ($vars[$j] eq '0'))
                {
                    $vars[$i]="BS_AND($vars[$i],BS_NOT(b_$j))";
                    $vars[$j]='';
                }
                if (($vars[$i] eq '1') and (not $vars[$j] eq ''))
                {
                    $vars[$i]="BS_AND(b_$i,$vars[$j])";
                    $vars[$j]='';
                }
                if (($vars[$i] eq '0') and (not $vars[$j] eq ''))
                {
                    $vars[$i]="BS_AND(BS_NOT(b_$i),$vars[$j])";
                    $vars[$j]='';
                }
            }
        }

        # debug print 
        # now array contains no more '0' '1' - combine remaining terms
        for (my $k=0; $k<4; $k++)
        {
            for (my $i=0; $i<7; $i++)
            {
                print "$k  vars[$i]  $vars[$i]\n" if $debug;
            }
            for (my $i=0; $i<7; $i++)
            {
                for (my $j=$i+1; $j<8; $j++)
                {
                    if (not ($vars[$i] eq '') and (not $vars[$j] eq ''))
                    {
                        $vars[$i]="BS_AND($vars[$i],$vars[$j])";
                        $vars[$j]='';
                        print "found bs pair in $i $j\n" if $debug;
                        $i++;
                    }
                }
            }
        }
        
        for (my $i=0; $i<8; $i++)
        {
            print"$i:  $vars[$i]\n" if $debug;
        }

        #$line = "\n /*$term*/ ";
        $line = "";
        for (my $i=0; $i<8; $i++)
        {
            $line = $line.$vars[$i];
        }
        # $line contains now resolved string of one minterm
        $outterms[$outterm]=$line; $outterm++;
    }

    for (my $i=0; $i<100; $i++) {print "outterm[$i] $outterms[$i]\n" if $debug;}
    # all terms are in @outterms, now OR them together
    my $lastindex=0;
    for (my $k=0; $k<10; $k++)
    {
        for (my $i=0; $i<100; $i++)
        {
            for (my $j=$i+1; $j<101; $j++)
            {
                if (not ($outterms[$i] eq '') and (not $outterms[$j] eq ''))
                {
                    $outterms[$i]="BS_OR($outterms[$i],$outterms[$j])"; $lastindex=$i;
                    $outterms[$j]='';
                    $i++;
                }
            }
        }
    }

    for (my $i=0; $i<100; $i++) {print "outterm[$i] $outterms[$i]\n" if $debug;}
    
    print "out[$outbit] = $outterms[$lastindex];\n";

    for (my $k=0; $k<100; $k++)
    {
        $outterms[$k]='';
    }
    $outbit++;
}