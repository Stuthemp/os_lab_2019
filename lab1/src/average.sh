#!/bin/bash

exec 0<numbers

sum=0
count=0

while read number
do

sum=$(($sum+$number))
count=$(($count+1))

done

average=$(($sum/$count))

echo "Среднее значение= $average" 
echo "Число элементов= $count"