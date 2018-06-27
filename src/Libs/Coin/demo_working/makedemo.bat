scc demo.c
echo lo demo.obj >link.in
echo lo u1.dll >>link.in
echo fi demo.exe >>link.in
slink link.in

