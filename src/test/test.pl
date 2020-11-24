
my $help = <<END;
MiniDB test suite
-----------------

Usage: perl test.pl [options]

Options:
   -iters <int n>  Run all [tests] n times
   -h, -help:      Show this help text
   -t, -test:      Adds test to the list of tests to be run
   -p, -ptest:     Run a perl/python script that tests MiniDBShell.
                   Useful for random fuzzers, etc.
                   Format: test-name.ext,arg1,arg2,...,argn
   -keep:          Keep results of passed tests

No options: run all the tests in the directory <tests>

Example: perl test.pl -test mytest -ptest myptest.py,1,2,3 -iters 4 --keep

A test should be the name of a file with one of the following extensions:
   - .test: if this extension is found, all .sql or .cmnd of the same name will
            be ignored.
   - .sql: use this extension if you want to test a SQL query.
   - .cmnd: use this extension if you want to test MiniDBShell commands.
   - .sql, .cmnd: if both extensions are found with the same name, first the .sql
                  will be executed and then the .cmnd.

A perl/python test should print either "passed" or "success" when it finishes.
This scripts will receive in their argv, the arguments passed from the command
line (as shown in the example) and the index of the iter in which this is being run.
For example: "-ptest a.py,1000" and it's the second time this test is being run,
             then argv = [1000, 2]

Optional environment variables:
  - MINIDB_ROOT: this is where MiniDBShell executable can be found.
                 Default: <this script path>/..
  - MINIDB_TEST: this is the directory where the directories: <tests> and <results>
                 can be found. Default: <this script path> (no ending slash).

Note: If a file with name "ignore-*.sql" is found, it will be ignored.
      Use this type of file if you want to have many calls to loadQuery with different
      .sql files as argument in a cmnd file.

END

use strict;
use warnings;
use File::Compare;
use File::Copy;
use Getopt::Long;
use FindBin '$Bin';

sub printTime(@);
sub cleanOutput(@);
sub printTestList(@);

my @tests;
my %ptests;
my %results;
my $elapsed = 0;
my $iters = 1;
my $keep = 0;

###################################################
# Get important directories: root, tests, results #
###################################################

my $dir = (defined $ENV{MINIDB_TEST})? $ENV{MINIDB_TEST} : $Bin;
(my $root = $dir) =~ s/\/[^\/]+$//;
$root = $ENV{MINIDB_ROOT} if defined $ENV{MINIDB_ROOT};
my $minidb  = "$root/MiniDBShell";
my $resultdir = "$dir/results";
my $testdir = "$dir/tests";

$ENV{MINIDB_TEST} = $dir;
$ENV{MINIDB_ROOT} = $root;

print "MiniDBShell executable: $minidb\n";
print "Results directory: $resultdir\n";
print "Test files directory: $testdir\n";

###################################
# Get CL options and tests to run #
###################################

GetOptions(
  'h|help'	=> sub { print $help; exit(0); },
  'iters=i'	=> sub { $iters = $_[1]; },
  'test|t=s'	=> sub { push @tests, $_[1]; },
  'ptest|p=s@'	=> sub { my @d = split ",", $_[1];
                         @{$ptests{$d[0]}} = @d[1..$#d] if $#d > 0;
                         $ptests{$d[0]} = [] if $#d == 0;
                       },
  'keep'	=> sub { $keep = 1; }
) or die $help;

if (scalar @tests == 0 and scalar keys %ptests == 0) {
  opendir DIR, $testdir or die $!;
  my $common_match = sub { -f "$testdir/$_" && $_ !~ /^ignore-/ };
  my @plpy = grep { $common_match->($_) && $_ =~ /\.(pl|py)$/ } readdir DIR;
  opendir DIR, $testdir or die $!;
  @tests = grep { $common_match->($_) && s/\.(sql|cmnd|test)$// } readdir DIR;
  for my $test (@plpy) {
    $ptests{$test} = [];
  }
  closedir DIR;
}

print "The following tests will be run: ";
print join(", ", (@tests, keys %ptests)) . "\n\n";

#############
# Run tests #
#############

unless (-e $resultdir or mkdir $resultdir) {
  die "Unable to create results directory ($resultdir)";
}

for (my $i = 1; $i <= $iters; $i++) {

  foreach my $test (keys %ptests) {
    my $args = "";
    $args = join(' ', @{$ptests{$test}}) if scalar @{$ptests{$test}} > 0; 
    $args .= " i$i";
    my $lang = ($test =~ /\.py$/)? 'python' : 'perl';
    my $ext = ($test =~ /\.py/)? 'py' : 'pl';
    my $test_i = "$test"."_$i";
    my $output = ($iters == 1)? "$resultdir/$test" : "$resultdir/$test_i";
    print "Running $lang test: $test\n";
    $test =~ s/\.(pl|py)//;

    my $start = time();
    system("$lang $testdir/$test.$ext $args > $output.out");
    $elapsed += time() - $start;

    (my $result = lc `tail -1 $output.out`) =~ s/^\s+|\s+$//g;
    $results{"$test_i"} = $result;
    print "$test $result\n\n";
    if (not $keep) {
      unlink "$output.out" if $result eq "passed";
    }
  }

  foreach my $test (@tests) {
   my $test_i = "$test"."_$i";
   my $output = ($iters == 1)? "$resultdir/$test" : "$resultdir/$test_i";
   copy("$testdir/$test.mas", "$output.mas"); 

    print "Running: $test\n";
    my $cmd = "$minidb";
    if (-e "$testdir/$test.test") {
      $cmd = "$cmd -f $testdir/$test.test" if -e "$testdir/$test.test";
    } else {
      $cmd = "$cmd -q $testdir/$test.sql" if -e "$testdir/$test.sql";
      $cmd = "$cmd -f $testdir/$test.cmnd" if -e "$testdir/$test.cmnd";
    }

    my $start = time();
    system("$cmd -e saveDB $output"."_DB exit > $output.out");
    $elapsed += time() - $start;

    cleanOutput("$output.out");
    cleanOutput("$output.mas");  

    $results{"$test_i"} = !compare("$output.out", "$output.mas")? "passed" : "failed";
    if ($results{"$test_i"} eq "failed") {
      print "Saving differences...\n";
      system("diff $resultdir/$test.mas $output.out > $output.dif");
    } elsif (not $keep) {
      unlink "$output"."_DB.zip";
      unlink "$output.mas";
      unlink "$output.out";
    }
    print "$test $results{$test_i}\n"; 
    print "\n";
  }
}

################
# Clean files  #
################

for (my $i = 0; $i < 10; $i++) {
  system("rm t$i* 2>/dev/null");
  system("rm $i* 2>/dev/null");
}

system("rm *.zip 2>/dev/null");

################# 
# Print results #
#################

print "RESULTS\n";
print "=======\n";

my @passed_tests = grep {$results{$_} eq "passed"} keys %results; 
my @failed_tests = grep {$results{$_} eq "failed"} keys %results;

print "@{[scalar keys %results]} tests run, ";
print "@{[scalar @passed_tests]} passed and ";
print "@{[scalar @failed_tests]} failed in ";
printTime $elapsed;

printf "\nPassed (%.2f%%):\n", 100 * scalar @passed_tests / scalar keys %results;
printTestList @passed_tests;

printf "\nFailed (%.2f%%):\n", 100 * scalar @failed_tests / scalar keys %results;
printTestList @failed_tests;

print "\n";

###############
# Subroutines #
###############

# Print n seconds as: hours minutes seconds
sub printTime(@) {
  use integer;
  my $seconds = $_[0];
  my $hours = $seconds / 3600;
  my $minutes = ($seconds % 3600) / 60;
  $seconds = ($seconds % 3600) % 60;
  print "$hours hours $minutes minutes and $seconds seconds\n";
}

# Remove useless MiniDBShell output in the given file\
# to make it more readable
sub cleanOutput(@) {
  my $filename = $_[0];
  my $file;
  open $file, '<', $filename or die "error opening $filename: $!";
  my $data = do { local $/; <$file> };
  close $file;

  # Remove MiniDB CLI output
  $data =~ s/Welcome!!!!|Query:|Parse SUCCESS!!|Enter Command://g;

  # Remove SaveDB output
  $data =~ s/DB was saved properly!!//g;
  $data =~ s/(adding|updating):.*\)\n//g;

  # Remove useles spaces
  $data =~ s/ +/ /g;

  # Trim
  $data =~ s/^\s+|\s+$//g;

  # Ugly hardcoded fix for magic spaces
  $data =~ s/ *\n */\n/g;
  $data =~ s/' (\d+\.\d+)'/'$1'/g;

  # Remove useless \n
  $data =~ s/\n{3,}/\n\n/g;

  open $file, ">$filename" or die "error opening $filename: $!";
  print $file $data;
  close $file;
}

# Print the given test name list
sub printTestList(@) {
  my @tests = @_;
  if (scalar @tests > 0) {
    print "    $_\n" for map {local $_ = $_; s/(.+)_(\d+)$/\($2\) $1/; $_} @tests;
  } else {
    print "    None\n";
  }
}
