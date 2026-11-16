#!/bin/bash
#How to use: ./replace_Trise.sh Conf/00A/PreprocessingFilterPSA.conf 250
thepath=`pwd`
thefile=$1
thevalue=$2

thepfile=${thepath}/${thefile}


if ! test ${thepfile} ; then
	exit 0
fi


expand -t 4 $thepfile > .tmpfile.txt
#echo "the new"


theoldvalue=`head -n1 .tmpfile.txt | awk '{print $4}' `
echo "Replacing Trise $theoldvalue with $thevalue" 

sed -i -e "s/\ ${theoldvalue}\ /\ ${thevalue}\ /g" .tmpfile.txt 
#echo "sed -i -e "s/\ ${theoldvalue}\ /\ ${thevalue}\ /g" .tmpfile.txt "

mv .tmpfile.txt $thepfile
#rm .tmpfile.txt
