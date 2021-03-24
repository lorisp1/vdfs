#!/bin/bash

# command reassignment for script subshell
# *** for more info see documentation ***
shopt -s expand_aliases;
alias ls='ls_mt';
if [ -e /opt/vd.fun ]; then
	source /opt/vd.fun;
else
	source $HOME/.vd/vd.fun;
fi
# end of command reassignment

# echo "vd start";
# vd start ;
# echo "Press <return> to continue...";
# read go;

echo "mkdir VD:/topi";
mkdir VD:/topi;
echo "Press <return> to continue...";
read go;

echo "mkdir VD:/paperi";
mkdir VD:/paperi;
echo "Press <return> to continue...";
read go;

echo "mkdir VD:/topi/mikey";
mkdir VD:/topi/mikey;
echo "Press <return> to continue...";
read go;

echo "cp ./files_in/try_small.txt VD:/";
cp ./files_in/try_small.txt VD:/;
echo "Press <return> to continue...";
read go;

echo "cp ./files_in/try_small.txt VD:/topi/newname";
cp ./files_in/try_small.txt VD:/topi/newname;
echo "Press <return> to continue...";
read go;

echo "cp VD:/try_smal VD:/topi/mikey";
cp VD:/try_smal VD:/topi/mikey;
echo "Press <return> to continue...";
read go;

echo "cp VD:/try_smal VD:/topi/mikey";
cp VD:/try_smal VD:/topi/mikey;
echo "Press <return> to continue...";
read go;

echo "mkdir VD:/paperi/paolino";
mkdir VD:/paperi/paolino;
echo "Press <return> to continue...";
read go;

echo "mkdir VD:/paperi/nonna";
mkdir VD:/paperi/nonna;
echo "Press <return> to continue...";
read go;

echo "ln -s VD:/paperi/zio VD:/ricco";
ln -s VD:/paperi/zio VD:/ricco;
echo "Press <return> to continue...";
read go;

echo "ln VD:/topi/newname VD:/hardl";
ln VD:/topi/newname VD:/hardl;
echo "Press <return> to continue...";
read go;

echo "ls VD:/";
ls VD:/;
echo "Press <return> to continue...";
read go;

echo "cp VD:/try_smal VD:/topi/mikey";
cp VD:/try_smal VD:/topi/mikey;
echo "Press <return> to continue...";
read go;

echo "cp ./files_in/try_big.txt VD:/topi/mikey/newname";
cp ./files_in/try_big.txt VD:/topi/mikey/newname;
echo "Press <return> to continue...";
read go;

echo "ls VD:/topi/mikey/newname";
ls VD:/topi/mikey/newname;
echo "Press <return> to continue...";
read go;

echo "del VD:/topi/mikey/newname";
del VD:/topi/mikey/newname;
echo "Press <return> to continue...";
read go;

echo "ls VD:/topi/mikey/";
ls VD:/topi/mikey/;
echo "Press <return> to continue...";
read go;

echo "ls -a VD:/topi/mikey/";
ls -a VD:/topi/mikey/;
echo "Press <return> to continue...";
read go;

echo "undel VD:/topi/mikey/newname";
undel VD:/topi/mikey/newname;
echo "Press <return> to continue...";
read go;

echo "ls VD:/topi/mikey/";
ls VD:/topi/mikey/;
echo "Press <return> to continue...";
read go;

echo "del VD:/topi/mikey/newname";
del VD:/topi/mikey/newname;
echo "Press <return> to continue...";
read go;

echo "ls -a VD:/topi/mikey/";
ls -a VD:/topi/mikey/;
echo "Press <return> to continue...";
read go;

echo "purge VD:/topi/mikey/newname";
purge VD:/topi/mikey/newname;
echo "Press <return> to continue...";
read go;

echo "ls -a VD:/topi/mikey/";
ls -a VD:/topi/mikey/;
echo "Press <return> to continue...";
read go;

echo "cp VD:/try_smal ./files_out/";
cp VD:/try_smal ./files_out/;
echo "Press <return> to continue...";
read go;

echo "cp VD:/try_smal ./files_out/renamed";
cp VD:/try_smal ./files_out/renamed;
echo "Press <return> to continue...";
read go;

echo "ls -a VD:/";
ls -a VD:/;
echo "Press <return> to continue...";
read go;

echo "ls VD:/topi/mikey";
ls VD:/topi/mikey;
echo "Press <return> to continue...";
read go;

echo "rm VD:/ricco";
rm VD:/ricco;
echo "Press <return> to continue...";
read go;

echo "ls VD:/";
ls VD:/;
echo "Press <return> to continue...";
read go;

echo "rm VD:/hardl";
rm VD:/hardl;
echo "Press <return> to continue...";
read go;

echo "ls VD:/";
ls VD:/;
echo "Press <return> to continue...";
read go;

echo "rm VD:/hardl";
rm VD:/hardl;
echo "Press <return> to continue...";
read go;

echo "ls VD:/hardl";
ls VD:/hardl;
echo "Press <return> to continue...";
read go;

echo "vd stop";
vd stop;
echo "Press <return> to continue...";
read go;
