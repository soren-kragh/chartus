my $short = 0;
my $full = 0;

while (<>) {
  if ( m/^## Shortlist of All Specifiers$/ ) {
    $short = 1;
  }
  if ( m/^## Full Documentation$/ ) {
    $full = 1;
  }
  if ( m/^```$/ ) {
    print;
    if ( $short == 1 ) {
      $short = 2;
      print `./chartus -t`;
      next;
    }
    if ( $full == 1 ) {
      $full = 2;
      print `./chartus -T`;
      next;
    }
    $short = 0;
    $full = 0;
    next;
  }
  next if ( $short == 2 || $full == 2 );
  print;
}
