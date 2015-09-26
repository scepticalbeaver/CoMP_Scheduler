#!/bin/bash
# script for post simulation and printing results

clear
rm -f compAlgo/output/*
cp testsDir/build/DlRlcStats.txt compAlgo/input/.
cp testsDir/build/measurements.log compAlgo/input/.

if [ -z "$1" ]
then
	gnuplot -p testsDir/build/enbs.txt testsDir/build/ues.txt  remPlotScript.p	
	echo "Radio environment plot done. See 'SceneX.png'"
fi


cd compAlgo
echo "Compiling CoMP simulation app"
qmake DEFINES+="NDEBUG" && make -j4 --quiet || exit 1
echo "" && echo ""

./build/release/bin/compAlgo || (echo "Simulation failed" && exit)
cd ..

echo ""
echo "Undetermined results:"
python throughputCalc.py compAlgo/input/DlRlcStats.txt

echo "CoMP algo:"
python throughputCalc.py compAlgo/output/DlRlcStats.txt --ignore-imsi
echo ""

python plotRsrp.py --ue $1
echo "Measurements plot done. See 'measurements_plot.png'"
echo "Postprocessing finished"

cd compAlgo
make --quiet clean
echo "Simulation finished"

