#!/bin/bash

export HEADER='\033[95m'
export OKBLUE='\033[94m'
export OKCYAN='\033[96m'
export OKGREEN='\033[92m'
export RED='\e[1m\e[38;5;196m'
export NC='\e[0m'
export WARNING='\033[93m'
export FAIL='\033[91m'
export ENDC='\033[0m'
export BOLD='\033[1m'
export UNDERLINE='\033[4m'


source /agata/topology/agatavars
unset currentDets

 
######### list of detectors to be used (copy from nearline if not present)
        currentDets=$AGATA_CRYSTALS
	export currentDets
        currentDets=$(echo "$currentDets" | tr ' ' '\n' | sort -t ' ' -k1,1 | tr '\n' ' ')
	echo -e $HEADER"Using dets from /agata/topology/agatavars: ${OKBLUE} currentDets= $currentDets  " $ENDC 
	

#check if ExpName variable is defined
if [[ -z $ExpName ]] ; then
	echo -e $FAIL"ExpName variable not defined, did you forget t EXP_...?"$ENDC
	return 1
fi

#check if scriptspath variable is (correcly) defined
scriptspath=/opt/data/LNL/${ExpName}/AgaPrep/SCRIPTS_test
if ! test -d $scriptspath ; then
	echo -e $FAIL"Cannot find: $scriptspath "$ENDC
	return 1
else 
	echo -e $HEADER"Scripts path: $scriptspath "$ENDC
	if [[ ":$PATH:" == ":$scriptspath:" ]] ; then 
		echo "already in path"
		continue
	else 
		echo -e $HEADER"Adding scripts folder to path"$ENDC
		PATH=$PATH:$scriptspath
	fi
fi


echo $PWD

