set term png size 700, 650
set output "sceneX.png"
set view map
set tics font ", 16"
set xlabel "X [m]" font ", 18"
set ylabel "Y [m]" font ", 20"
set cblabel "SINR   [dB]" font ", 20"
set title "Radio Environment Map" font ", 20"
unset key
plot "./testsDir/build/scene1.rem" using ($1):($2):(10*log10($4)) with image

