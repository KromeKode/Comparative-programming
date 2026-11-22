module example.com/stats

go 1.23

require example.com/textstats v1.0.0

// Use the module in ./textstats instead of downloading example.com/textstats.
replace example.com/textstats => ./textstats
