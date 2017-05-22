#! /usr/bin/perl

my $arg = $ARGV[0];

sub randomRange {
  my $num1 = shift;
  my $num2 = shift;
  return int( (rand() * ($num2 - $num1 + 1)) + $num1 );
}

`echo "" > /tmp/app.log` unless $arg;

open my $fp, ">> /tmp/app.log";
for (1..1000000) {
    my $main_key = "main";
    my $val = $_;
    my $txt = sprintf "%s:%d\n", $main_key, $val;
    print $fp $txt;
#    print $txt."\n";
}
close $fp;
for (1..33) {
    my $sub_key = "sub";
    my $val = $_;
    my $txt = sprintf "%s:%d", $sub_key, $val;
    `echo $txt >> /tmp/app.log`;
    print $txt."\n";
}
for (1..10) {
    my $sub_key = "sub_type1";
    my $val = $_;
    my $txt = sprintf "%s:%d", $sub_key, $val;
    `echo $txt >> /tmp/app.log`;
    print $txt."\n";
}
for (1..10) {
    my $sub_key = "sub_type2";
    my $val = $_;
    my $txt = sprintf "%s:%d", $sub_key, $val;
    `echo $txt >> /tmp/app.log`;
    print $txt."\n";
}
for (1..10) {
    my $sub_key = "hoge";
    my $val = $_;
    my $txt = sprintf "%s:%d", $sub_key, $val;
    `echo $txt >> /tmp/app.log`;
    print $txt."\n";
}

# try to input some keys that don't exist in main keys
for (1000001..1000010) {
    my $sub_key = "hoge";
    my $val = $_;
    my $txt = sprintf "%s:%d", $sub_key, $val;
    `echo $txt >> /tmp/app.log`;
    print $txt."\n";
}
