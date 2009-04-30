%fingerprint-spec 1.4
%fprint:DATE
%url:http://rcfunge98.com/rcsfingers.html#DATE
%desc:Date Functions
%condition:!defined(CFUN_NO_FLOATS)
%safe:true
%begin-instrs
#I Name              Desc
A  add_days          Add days to date
C  jdn_to_ymd        Convert Julian day to calendar date
D  day_diff          Days between dates
J  ymd_to_jdn        Calendar date to Julian day
T  year_day_to_full  Year/day-of-year to full date
W  week_day          Day of week (0=Monday)
Y  year_day          Day of year (0=Jan 1)
%end
