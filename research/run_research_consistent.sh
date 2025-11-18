#!/bin/bash

# Создаем директорию для результатов, если она не существует
mkdir -p research/out

# Базовые параметры
MIN_DURATION=1.0
MAX_DURATION=15.0
EXCHANGE_INTERVAL=100
INITIAL_TEMP=100.0
COOLING_LAW="boltzmann"
ITERATIONS_PER_TEMP=100
MAX_ITER_NO_IMPROVE=100
MAX_ITER_NO_IMPROVE_GLOBAL=1
NUM_THREADS=1

# Функция для запуска программы с сохранением вывода
run_experiment() {
    local jobs=$1
    local procs=$2
    local iter=$3
    local label="${jobs}task_${procs}proc_${iter}iter"
    local output_file="research/out/constistent_experiment_${label}.txt"
    
    echo "Запуск эксперимента: jobs=$jobs, processors=$procs, iter=$iter"
    
    ./AnnealingScheduler $jobs $procs $MIN_DURATION $MAX_DURATION \
        $EXCHANGE_INTERVAL $INITIAL_TEMP $COOLING_LAW $ITERATIONS_PER_TEMP \
        $MAX_ITER_NO_IMPROVE $MAX_ITER_NO_IMPROVE_GLOBAL $NUM_THREADS > $output_file
    
    echo "Результаты сохранены в: $output_file"
    cat $output_file
    echo "----------------------------------------"
}

for task in 100 200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400 1500 1600 1700 1800 1900 2000; 
do 
    for proc in 2 4 6 8 10 12 14 16 18 20;
    do 
        for iter in 1 2 3 4 5;
        do 
            run_experiment $task $proc $iter
        done
    done
done

echo "Все эксперименты завершены!"