set term png size 720, 580
set output "sceneX.png"
set view map
set xlabel "X"
set ylabel "Y"
set cblabel "SINR (dB)"
set title "Radio Environment Map"
unset key
plot "./testsDir/build/debug/bin/scene1.rem" using ($1):($2):(10*log10($4)) with image

