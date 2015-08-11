set term png
set output "sceneX.png"
set view map
set xlabel "X"
set ylabel "Y"
set cblabel "SINR (dB)"
unset key
plot "./build/debug/bin/scene1.rem" using ($1):($2):(10*log10($4)) with image

