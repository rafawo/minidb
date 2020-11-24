
use strict;
use warnings;

open SQL_FILE, ">tests/chars.sql" or die "Couldn't open sql file";
open MAS_FILE, ">tests/chars.mas" or die "Couldn't open sql file";

my $queries = "create table t1(a char);\n";
my $result = "Welcome!!!!\n\nQuery:\ncreate table t1(a char);\n\n\nParse SUCCESS!!\n\n";

for (my $i = 0; $i < 256; $i++) {
  my $c = chr($i);
  $queries .= "insert into t1 values('$c');\n";
  $result .= "Query:\ninsert into t1 values('$c');\n\n\nParse SUCCESS!!\n\n";
}

$queries .= "select * from t1;";
$result.= "Query:\nselect * from t1;\n\n";
$result .= "Table Name: \@temp_table\nNumber of Columns: 1\nRow Size: 1 bytes\n\n";
$result .= "Column DataTypes:\na(char)\n";

for (my $i = 0; $i < 256; $i++) {
  my $c = chr($i);
  $result .= "'$c'\n";
}

$result .= "\nParse SUCCESS!!";

print SQL_FILE $queries;
print MAS_FILE $result;

close SQL_FILE;
close MAS_FILE;
