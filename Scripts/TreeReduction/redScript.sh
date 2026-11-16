#!/bin/bash

for i in 0031 0032 0033 0034
do
	./redSingleRun.sh $i &
done

wait
echo "Batch done"

for i in 0035 0036 0037 0038
do
	./redSingleRun.sh $i &
done

wait
echo "Batch done"

for i in 0039 0040 0041 0042
do
	./redSingleRun.sh $i &
done

wait
echo "Batch done"

for i in 0043 0044 0045 0046 0047
do
	./redSingleRun.sh $i &
done

wait
echo "Batch done"

echo "Finished"
