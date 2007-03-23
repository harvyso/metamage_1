#!/usr/bin/jtest

$ sh /proc/self/fd/3

? 1

3 << 'END'
set -e
echo pretty
false
echo bad
END

1 >= pretty

%%

$ sh /proc/self/fd/3

? 0

3 << 'END'
set -e
set +e
false
echo good
END

1 >= good

