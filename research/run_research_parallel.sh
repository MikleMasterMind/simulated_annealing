#!/bin/bash

# Создаем папку для результатов
mkdir -p research/out

# Параметры алгоритма
JOBS=5000
PROCESSORS=20
MIN_DURATION=1.0
MAX_DURATION=15.0
EXCHANGE_INTERVAL=100
INITIAL_TEMP=100.0
COOLING_LAW="boltzmann"
ITERATIONS_PER_TEMP=100
MAX_ITER_NO_IMPROVE=100
MAX_ITER_GLOBAL=1
THREAD_COUNTS=(1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16)
ITERATIONS=5

for threads in "${THREAD_COUNTS[@]}"; do
    for ((iter=1; iter<=ITERATIONS; iter++)); do

        echo "threads=$threads iter=$iter"
        
        OUTPUT_FILE="research/out/parallel_experiment_${threads}thread_${iter}iter.txt"
        
        ./AnnealingScheduler $JOBS $PROCESSORS $MIN_DURATION $MAX_DURATION \
                            $EXCHANGE_INTERVAL $INITIAL_TEMP $COOLING_LAW \
                            $ITERATIONS_PER_TEMP $MAX_ITER_NO_IMPROVE $MAX_ITER_GLOBAL \
                            $threads > "$OUTPUT_FILE"
        
        
        FITNESS=$(grep "Best solution fitness" "$OUTPUT_FILE" | tail -1 | awk '{print $4}')
        TIME_MS=$(grep "Algorithm completed" "$OUTPUT_FILE" | awk '{print $4}')
        IMPROVEMENT=$(grep "Improvement:" "$OUTPUT_FILE" | awk '{print $2}')
        
        echo "    Fitness: $FITNESS, Time: ${TIME_MS}ms, Improvement: $IMPROVEMENT"
        
        sleep 1
    done
    
    echo ""
done