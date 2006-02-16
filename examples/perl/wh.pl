# Copyright (C) 2004-2006 Ryan McCabe <ryan@numb.org> (GPL v2)
# This is very useful.

sub wh {
	my $maxlen = 0;
	my @toks = split(/ /, shift);

	foreach my $i (@toks) {
		my $len = length($i);
		$maxlen = $len if ($len > $maxlen);
	}

	foreach my $i (@toks) {
		PORK::input_send(" " x ($maxlen - length($i)) . join(' ', split(//, uc($i))));
	}
}

PORK::alias("wh", "perl wh");
