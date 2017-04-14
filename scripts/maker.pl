#! /usr/bin/perl

my $arg = $ARGV[0];

sub randomRange {
  my $num1 = shift;
  my $num2 = shift;
  return int( (rand() * ($num2 - $num1 + 1)) + $num1 );
}

`echo "" > /tmp/app.log` unless $arg;

for (0..10000) {
    my $rnd = randomRange(1,10);
    my $prefix = $arg ? $arg : ($rnd <= 3 ? "b" : "a");
    my $v = sprintf "%s:%d", $prefix, randomRange(1,100);
    `echo $v >> /tmp/app.log`;
    print $v."\n";
}
