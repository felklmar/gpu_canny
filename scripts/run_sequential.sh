#!/bin/bash

for i in {1..10}
do
    ./../canny_sequential images/wiedzmin.tga 1.5 .1 .2 >>output.txt
done