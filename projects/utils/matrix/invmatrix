#!/bin/bash

echo "file = \"$1\"
l = BinaryReadList[file <> \".matrix\", \"Real64\"];
size = Sqrt[Length[l]]
m = Table[l[[size*i + j]], {i, 0, size - 1}, {j, 1, size}];
im = Inverse[m];
(*ArrayPlot[m]
ArrayPlot[im]*)

str = OpenWrite[file <> \".inverse\", BinaryFormat -> True];
BinaryWrite[str, im, {\"Real64\"}];
Close[str];
" | math
