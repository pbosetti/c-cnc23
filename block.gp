#    ____                   _       _   
#   / ___|_ __  _   _ _ __ | | ___ | |_ 
#  | |  _| '_ \| | | | '_ \| |/ _ \| __|
#  | |_| | | | | |_| | |_) | | (_) | |_ 
#   \____|_| |_|\__,_| .__/|_|\___/ \__|
#                    |_|                
# Example plot script

# First set output format and file:
# set terminal pngcairo size 800, 600
# set output "plot.png"

# then set properties:
set xlabel 'time (s)'
set grid

# finally decide what to plot:
set multiplot layout 2,1
set ylabel 'lambda'
plot "block.txt" using 1:2 with lines title "lambda"
set ylabel 'feedrate (mm/s)'
plot "block.txt" using 1:3 w lp t "feedrate"
unset multiplot
