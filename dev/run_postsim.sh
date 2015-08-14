# script for post simulation and printing results

rm -f compAlgo/output/*
cp testsDir/build/debug/bin/DlMacStats.txt compAlgo/input/.
cp testsDir/build/debug/bin/measurements.log compAlgo/input/.

gnuplot -p testsDir/build/debug/bin/enbs.txt testsDir/build/debug/bin/ues.txt  remPlotScript.p
echo "Radio environment plot done. See 'SceneX.png'"

cd compAlgo
./build/debug/bin/compAlgo
cd ..
echo "Simulation finished"

echo ""
echo "Undetermined results:"
python throughputCalc.py compAlgo/input/DlMacStats.txt

echo "CoMP algo:"
python throughputCalc.py compAlgo/output/DlMacStats.txt --ignore-imsi
echo ""

python plotRsrp.py --rm-outliers --ue
echo "Measurements plot done. See 'measurements_plot.png'"
echo "Postprocessing finished"

