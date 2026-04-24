#!/bin/bash

block2D_x=(8 15 16 32 32 8)
block2D_y=(8 15 16 32 8 32)
block1D=(32 128 256 1024)

i_max=$((${#block2D_x[@]} - 1))
j_max=$((${#block1D[@]} - 1))

runs=9

for i in `seq 0 $i_max`
do
    for j in `seq 0 $j_max`
    do
        for run in `seq 0 $runs`
        do
            ./../canny_cuda ../images/wiedzmin.tga 1.5 0.1 0.2 ${block2D_x[i]} ${block2D_y[i]} ${block1D[j]} >>output_cuda.txt
        done

        echo >>output_cuda.txt
    done
done