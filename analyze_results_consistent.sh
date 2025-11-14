#!/bin/bash

mkdir -p out
mkdir -p results

# Массивы с уникальными значениями jobs и processors
jobs_list=(100 200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400 1500 1600 1700 1800 1900 2000)
procs_list=(2 4 6 8 10 12 14 16 18 20)

# Создаем временный файл для данных
temp_file=$(mktemp)

# Собираем все данные во временный файл
for file in out/experiment_*task_*proc_*iter.txt; do
    filename=$(basename "$file")
    # Извлекаем jobs, processors и iteration из имени файла
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

# Создаем таблицу в CSV формате
echo -n "processors/jobs," > results/results_consistent.csv

# Записываем заголовок с jobs
for jobs in "${jobs_list[@]}"; do
    echo -n "$jobs," >> results/results_consistent.csv
done
echo "" >> results/results_consistent.csv  # новая строка

# Записываем данные: каждая строка - processors, затем средние значения времени
for proc in "${procs_list[@]}"; do
    echo -n "$proc," >> results/results_consistent.csv
    for jobs in "${jobs_list[@]}"; do
        # Получаем все времена выполнения для данной комбинации jobs/processors
        times=$(grep "^$jobs $proc " "$temp_file" | awk '{print $4}')
        
        if [ -n "$times" ]; then
            # Вычисляем среднее арифметическое
            count=0
            sum=0
            for time_val in $times; do
                sum=$((sum + time_val))
                count=$((count + 1))
            done
            if [ $count -gt 0 ]; then
                average=$((sum / count))
                echo -n "$average," >> results/results_consistent.csv
            else
                echo -n "," >> results/results_consistent.csv
            fi
        else
            echo -n "," >> results/results_consistent.csv
        fi
    done
    echo "" >> results/results_consistent.csv  # новая строка
done

# Удаляем временный файл
rm "$temp_file"

echo "Таблица результатов сохранена в results/results_consistent.csv"