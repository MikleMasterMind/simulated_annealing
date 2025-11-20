#!/bin/bash

outputfile="research/results_parallel.txt"

echo "Потоки | Среднее время (мс)" > "$outputfile"
echo "---------------------------" >> "$outputfile"

for threads in 1 2 4 8 16; do
    total_time=0
    count=0
    
    for iter in {1..5}; do
        file="research/out/parallel_experiment_${threads}thread_${iter}iter.txt"
        if [ -f "$file" ]; then
            time=$(grep "Algorithm completed in" "$file" | awk '{print $4}')
            total_time=$((total_time + time))
            count=$((count + 1))
        fi
    done
    
    if [ $count -gt 0 ]; then
        avg_time=$((total_time / count))
        echo "$threads      | $avg_time" >> "$outputfile"
    fi
done

echo >> "$outputfile"
echo "Потоки | Среднее улучшение (%)" >> "$outputfile"
echo "------------------------------" >> "$outputfile"

for threads in 1 2 4 8 16; do
    total_percentage=0
    count=0
    
    for iter in {1..5}; do
        file="research/out/parallel_experiment_${threads}thread_${iter}iter.txt"
        if [ -f "$file" ]; then
            # Заменяем запятую на точку и убираем символ %
            percentage=$(grep "Improvement percentage" "$file" | awk '{print $3}' | sed 's/,/./g' | sed 's/%//g')
            total_percentage=$(echo "$total_percentage + $percentage" | bc)
            count=$((count + 1))
        fi
    done
    
    if [ $count -gt 0 ]; then
        avg_percentage=$(echo "scale=4; $total_percentage / $count" | bc)
        echo "$threads      | $avg_percentage%" >> "$outputfile"
    fi
done
