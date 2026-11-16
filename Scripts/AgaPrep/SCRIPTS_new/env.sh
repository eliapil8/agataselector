#!/bin/bash

HEADER='\033[95m'
OKBLUE='\033[94m'
OKCYAN='\033[96m'
OKGREEN='\033[92m'
WARNING='\033[93m'
FAIL='\033[91m'
ENDC='\033[0m'
BOLD='\033[1m'
UNDERLINE='\033[4m'


######### list of detectors to be used
if [[ -z $currentDets ]]; then 
	currentDets=$(ssh prototype@172.16.40.120 'echo $AGATA_CRYSTALS') 
	echo -e $HEADER"Using dets from nearline1: ${OKBLUE}currentDets=\" $currentDets \" "$ENDC 
else  
	echo -e $HEADER"currentDets=${OKBLUE}\" $currentDets \" "$ENDC
fi


if [[ -z $currentDets ]]; then  
	echo -e $FAIL'Couldnt get automatically the list of dets using: currentDets=$(ssh prototype@172.16.40.120 echo $AGATA_CRYSTALS  '$ENDC
	return 1
fi 

if [[ -z $ExpName ]] ; then
	echo -e $FAIL"ExpName variable not defined, did you forget t EXP_...?"$ENDC
	return 1
fi


scriptspath=/opt/data/LNL/${ExpName}/AgaPrep/SCRIPTS_new
if ! test -d $scriptspath ; then
	echo -e $FAIL"Cannot found: $scriptspath "$ENDC
	return 1
else 
	echo -e $HEADER"Scripts path: ${OKBLUE}$scriptspath "$ENDC
	if [[ ":$PATH:" == ":$scriptspath:" ]] ; then 
		#echo "already in path"
		continue
	else 
		#echo -e $HEADER"Adding scripts folder to path"$ENDC
		PATH=$PATH:$scriptspath
	fi
fi


echo -e ${HEADER}"current directory: ${OKBLUE}$PWD"$ENDC

if $ready2go ; then echo -e $OKGREEN"Ready2go"$ENDC 
else echo -e $FAIL"Not ready"$ENDC
fi
