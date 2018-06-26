scc u1.c
echo dll >link.in
echo lo u1.obj >>link.in
echo exportall >>link.in
echo fi u1.dll >>link.in
slink link.in

