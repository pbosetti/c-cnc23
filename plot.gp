#    ____                   _       _   
#   / ___|_ __  _   _ _ __ | | ___ | |_ 
#  | |  _| '_ \| | | | '_ \| |/ _ \| __|
#  | |_| | | | | |_| | |_) | | (_) | |_ 
#   \____|_| |_|\__,_| .__/|_|\___/ \__|
#                    |_|                
# Example plot script

# First set output format and file:
set terminal pngcairo size 800, 600
set output "plot.png"

# then set properties:
set xlabel 'x'
set ylabel 'y'
set grid

# finally decide what to plot:
set multiplot layout 2,1
set xrange [0:7]
plot sin(x)+2 title "function", \
  "data.txt" using 1:2 with linespoints title "dataset"
plot cos(x) title "cosine"
unset multiplot
