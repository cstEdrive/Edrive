function [mati,ir]=rowitems(mat,it1,it2,it3,it4,it5,it6,it7,it8,it9)
% ROWITEMS  [mati,ir]=rowitems(mat,it1,it2,it3,it4,it5,it6,it7,it8,it9)
% returns rows of a string matrix that contains all input items.
% < mat: matrix of characters
% < it1: first item
% <  : :   :   item
% < it9: ninth item
% > mati: rows that match all items
% > ir  : indices of mat rows with full match: mati=mat(ir,:)
%
% Example
% >> mat=['<one> AB x '
%         '<one> AB y '
%         '<one>  C rz'
%         '<two> AB  x'
%         '<two> AB  y'
%         '<two> C  rz']
%    [mati,ir]=rowitems(mat,'<two>','y')
% mati =
%    <two> AB  y
% ir =
%     5
%
% It is also allowed to specify the items in a single string separated by blanks
% [mati,ir]=rowitems(mat,'<two> y')

% file           : ext5/rowitems.m
% author         : P.Wortelboer
% used functions : ext5/...
%                  finditem
%                  Wor/...
%                  strow2cl unblank
% last change    : 6 november 1998
% version        : 1e+

[nr,nc]=size(mat);
ir=[1:nr]';
if nargin==2 & any(it1==' ')
  it1=strow2cl(it1);
  ni=size(it1,1);
  allin1=1;
else
  allin1=0;
  ni=nargin-1;
end
mati=mat;
for i=1:ni
  if allin1
    it=unblank(it1(i,:));
  else
    eval(['it=it' int2str(i) ';'])
  end
  iri=finditem(mati,it);
  ir=ir(iri);
  mati=mati(iri,:);
end
 
% Copyright Philips CFT, 1998,  All Rights Reserved

