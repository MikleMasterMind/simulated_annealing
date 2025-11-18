#!/bin/bash

mkdir -p research/out

jobs_list=(100 200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400 1500 1600 1700 1800 1900 2000)
procs_list=(2 4 6 8 10 12 14 16 18 20)

temp_file=$(mktemp)

for file in research/out/constistent_experiment_*task_*proc_*iter.txt; do
    filename=$(basename "$file")
    jobs=$(echo "$filename" | sed 's/.*experiment_\([0-9]*\)task.*/\1/')
    processors=$(echo "$filename" | sed 's/.*task_\([0-9]*\)proc.*/\1/')
    iteration=$(echo "$filename" | sed 's/.*proc_\([0-9]*\)iter.*/\1/')
    
    execution_time=$(grep "Algorithm completed in" "$file" | sed 's/.*Algorithm completed in \([0-9]*\) ms.*/\1/')
    
    if [ -n "$execution_time" ] && [ -n "$jobs" ] && [ -n "$processors" ] && [ -n "$iteration" ]; then
        echo "$jobs $processors $iteration $execution_time" >> "$temp_file"
        echo "Обработан: $filename - $execution_time ms"
    else
        echo "Ошибка при обработке: $filename"
    fi
done

resultfile="research/results_consistent.csv"
echo -n "processors/jobs," > "$resultfile"

for jobs in "${jobs_list[@]}"; do
    echo -n "$jobs," >> "$resultfile"
done
echo "" >> "$resultfile"

for proc in "${procs_list[@]}"; do
    echo -n "$proc," >> "$resultfile"
    for jobs in "${jobs_list[@]}"; do
        times=$(grep "^$jobs $proc " "$temp_file" | awk '{print $4}')
        
        if [ -n "$times" ]; then
            count=0
            sum=0
            for time_val in $times; do
                sum=$((sum + time_val))
                count=$((count + 1))
            done
            if [ $count -gt 0 ]; then
                average=$((sum / count))
                echo -n "$average," >> "$resultfile"
            else
                echo -n "," >> "$resultfile"
            fi
        else
            echo -n "," >> "$resultfile"
        fi
    done
    echo "" >> "$resultfile"
done

rm "$temp_file"
