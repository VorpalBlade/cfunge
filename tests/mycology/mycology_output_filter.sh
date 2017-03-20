#!/bin/bash
# Filter for mycology output. Removes random and date dependent things.

exec \
	sed '/That the day of the month is/s/[0-9][0-9]*/#/' | \
	sed '/That the month is/s/[0-9][0-9]*/#/' | \
	sed '/That the year is/s/[0-9][0-9]*/#/' | \
	sed '/That the time is/s/[0-9][0-9]*/#/g' | \
	sed '/The directions were generated in the order/s/[<v>^]/#/g' | \
	sed '/? was met/s/[0-9][0-9]*/#/' | \
	sed '/UNDEF: S pushed [0-9]/s/[0-9][0-9]*/#/' | \
	sed '/UNDEF: T after M pushed /s/[0-9][0-9]*/#/g' | \
	sed '/UNDEF: called 3D 9 times and got /s/[0-9] /# /g' | \
	sed '/UNDEF: A pushed address /s/[0-9][0-9]*/#/g' | \
	sed '/UNDEF: YODHMS claim that the /s/[0-9][0-9]*/#/g' | \
	sed '/UNDEF: F claims that it is the/s/[0-9][0-9]*/#/g' | \
	sed '/UNDEF: W claims that it is the/s/[0-9][0-9]*/#/g'
