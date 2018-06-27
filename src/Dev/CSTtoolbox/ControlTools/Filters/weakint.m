function Cwi=weakint(fzero);
%lowpass filter
s=tf('s');
wzero=fzero*2*pi;
Cwi=(s+wzero)/s;
end %function