#!/bin/bash
# script for post simulation and printing results

clear
rm -f compAlgo/output/*
cp testsDir/build/debug/bin/DlRlcStats.txt compAlgo/input/.
cp testsDir/build/debug/bin/measurements.log compAlgo/input/.

gnuplot -p testsDir/build/debug/bin/enbs.txt testsDir/build/debug/bin/ues.txt  remPlotScript.p
echo "Radio environment plot done. See 'SceneX.png'"

cd compAlgo
echo "Compiling CoMP simulation app"
qmake DEFINES+="NDEBUG" && make -j4 --quiet || exit 1
echo "" && echo ""

./build/release/bin/compAlgo || (echo "Simulation failed" && exit 1)
make --quiet clean
cd ..
echo "Simulation finished"

echo ""
echo "Undetermined results:"
python throughputCalc.py compAlgo/input/DlRlcStats.txt

echo "CoMP algo:"
python throughputCalc.py compAlgo/output/DlRlcStats.txt --ignore-imsi
echo ""

python plotRsrp.py --ue $1
echo "Measurements plot done. See 'measurements_plot.png'"
echo "Postprocessing finished"
