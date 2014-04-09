


# 3D plot of FFT full, FFT oversampled, and DFT values.
set xrange[-250:250]
set yrange[-250:250]

set xlabel 'u'
set ylabel 'v'
set zlabel 'v2'

splot './fft_full.txt' using 3:4:(sqrt($5**2 + $6**2)) title 'fft', \
    './fft_oversampled.txt' using 2:3:(sqrt($4**2+$5**2)) every ::::528 title 'fft_oversampled', \
    './dft_output.txt' using 2:3:(sqrt($4**2 + $5**2)) every ::::528 title 'dft'

# V2 vs. baseline plot
reset
uv_mag(u, v) = sqrt(u**2 + v**2)
set yrange [0:1]
set xrange [0:250]
set xlabel '|UV|'
set ylabel 'V2'

plot './fft_oversampled.txt' using (uv_mag($2,$3)):(sqrt($4**2+$5**2)) every ::::528 title 'fft_oversampled', \
    './dft_output.txt' using (uv_mag($2,$3)):(sqrt($4**2 + $5**2)) every ::::528 title 'dft'



# Vis vs. baseline plot
reset
uv_mag(u, v) = sqrt(u**2 + v**2)
#set yrange [0:1]
set xrange [0:250]
set xlabel '|UV|'
set ylabel 'V2'
set ylabel 'Vis amp'
plot './fft_oversampled.txt' using (uv_mag($2,$3)):(sqrt($4**2+$5**2)) every ::::528 title 'fft_oversampled', \
    './dft_output.txt' using (uv_mag($2,$3)):(sqrt($4**2+$5**2)) every ::::528 title 'dft'


# phase vs. baseline plot
reset
uv_mag(u, v) = sqrt(u**2 + v**2)
#set yrange [0:1]
set xrange [0:250]
set xlabel '|UV|'
set ylabel 'Vis phase'
plot './fft_oversampled.txt' using (uv_mag($2,$3)):(atan2($5, $4)) every ::::528 title 'fft_oversampled', \
    './dft_output.txt' using (uv_mag($2,$3)):(atan2($5, $4)) every ::::528 title 'dft'


plot './fft_oversampled.txt' using (uv_mag($2,$3)):($4/sqrt($4**2+$5**2)) every ::::528 title 'fft_oversampled', \
    './dft_output.txt' using (uv_mag($2,$3)):($4/sqrt($4**2+$5**2)) every ::::528 title 'dft'



splot './fft_oversampled.txt' using 2:3:(atan2($5, $4)) every ::::528 title 'fft_oversampled', \
    './dft_output.txt' using 2:3:(atan2($5, $4)) every ::::528 title 'dft'



splot './fft_full.txt' using 3:4:(sqrt($5**2 + $6**2)) title 'fft'

reset
set xrange[-250:250]
set yrange[-250:250]

set xlabel 'u'
set ylabel 'v'
set zlabel 'imag'
splot './fft_full.txt' using 3:4:(atan2($6, $5)) title 'fft'
