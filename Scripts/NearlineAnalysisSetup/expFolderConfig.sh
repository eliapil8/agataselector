#!/bin/bash

DIR_EXP=/opt/data/LNL/$1
DIR_PRI=$DIR_EXP/PrismaAnalysis
DIR_ANA=$DIR_EXP/Analysis
DIR_SEL=$DIR_EXP/agataselector
DIR_REPLAY=$DIR_EXP/Replay
DIR_AGACAL=$DIR_EXP/AgaPrep
DIR_MERGE=$DIR_EXP/AncMerging

DIR_SEL_SRC=/opt/analysis-tools/LNL/agataselector/
DIR_FILTERS=/opt/analysis-tools/LNL/agataselector/

echo 
echo Configuring of experiment $1
echo 
if [ -d "$DIR_EXP" ];
then
	echo "$DIR_EXP exists"
else
	mkdir $DIR_EXP
fi

cd
source to_Workspace.sh $1

#agataselector folder
if [ -d "$DIR_SEL" ];
then
	echo "$DIR_SEL exists"
else
	mkdir $DIR_SEL
	cd $DIR_SEL
	cmake $DIR_SEL_SRC -DEXP_NAME=$1 -DBUILD_SCRIPTS=On -DUSE_MPI=On -DCMAKE_CXX_COMPILER=mpic++ 
	make -j12

	cp -v $DIR_SEL_SRC/User/EXP/$1/Conf/selector.conf $DIR_SEL_SRC/User/EXP/$1/Conf/selectorPrisma.conf
	cd $DIR_EXP
fi

echo "agataselector folder configured"
	
#AGATA replay folder
if [ -d "$DIR_REPLAY" ];
then
	echo "$DIR_REPLAY exists"
else
	mkdir $DIR_REPLAY
	cd $DIR_REPLAY
	ln -s $DIR_SEL_SRC/Scripts/Replay/replay.py
	cp -rv $DIR_SEL_SRC/Scripts/Replay/*.conf .
	cp -rv $DIR_SEL_SRC/Scripts/Replay/Template .
	cd $DIR_EXP
fi

echo "Replay folder configured"

#Prisma analysis folder
if [ -d "$DIR_PRI" ];
then
	echo "$DIR_PRI exists"
else
	mkdir $DIR_PRI
	cd $DIR_PRI

	ln -s /opt/analysis-tools/LNL/prismafilters/build/PrismaFilters
	
	ln -s $DIR_SEL_SRC/User/EXP/$1/Conf SelConf
	ln -s $DIR_SEL_SRC/User/EXP/$1/Conf/selectorPrisma.conf
	
	git clone https://baltig.infn.it/prisma/prismaonlinepackage.git
	g++ -Wall -pthread -O3 prismaonlinepackage/scripts/RunAnalysis/RunAnalysis.cxx -o RunAnalysis
	cp -rv prismaonlinepackage/scripts/RunAnalysis/Template/Runs . 
	cp -rv prismaonlinepackage/Conf/TemplateConf prismaonlinepackage/Conf/$1_Conf
	ln -s prismaonlinepackage/Conf/$1_Conf PrismaConf
	cp -v prismaonlinepackage/scripts/CheckCal/CheckCal.C .
	cp -v prismaonlinepackage/scripts/PrismaEfficiencies/PrismaEfficiencies.* .
	
	cd $DIR_EXP
fi

echo "PrismaAnalysis folder configured"

#Analysis folder for selector AGATA-ancillaries
if [ -d "$DIR_ANA" ];
then
	echo "$DIR_ANA exists"
else
	mkdir $DIR_ANA
	cd $DIR_ANA
	ln -s $DIR_SEL_SRC/User/EXP/$1/Conf
	ln -s $DIR_SEL_SRC/User/EXP/$1/Conf/selector.conf
	ln -s $DIR_REPLAY Data
	ln -s $DIR_PRI/Runs PrismaData
	ln -s $DIR_SEL/__MPI_RunSelector__
	echo ana01 > hfile
	echo ana02 >> hfile
	echo ana03 >> hfile
	echo ana04 >> hfile
	cd $DIR_EXP
fi

echo "Analysis folder configured"

#Ancillary merging folder
if [ -d "$DIR_MERGE" ];
then
	echo "$DIR_MERGE exists"
else
	mkdir $DIR_MERGE
	cd $DIR_MERGE
	ln -s ../agataselector/Scripts/AncMerging/ReadCaenRaw
	cd $DIR_EXP
fi

echo "AncMerging folder configured"

#AGATA calibrations and checks folder
if [ -d "$DIR_AGACAL" ];
then
	echo "$DIR_AGACAL exists"
else
	mkdir $DIR_AGACAL
	cd $DIR_AGACAL
	git clone https://baltig.infn.it/gongoras/agaselector.git
	echo "AgaSelector was cloned successfully."
	cp -rv $DIR_SEL_SRC/Scripts/AgaPrep/TemplatePrep .
	ln -s $DIR_SEL_SRC/Scripts/AgaPrep/SCRIPTS
	cd $DIR_EXP
fi

echo "AgaPrep folder configured"




