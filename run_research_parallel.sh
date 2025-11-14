#!/bin/bash

# Создаем папку для результатов
mkdir -p out

# Параметры алгоритма
JOBS=10000
PROCESSORS=20
MIN_DURATION=1.0
MAX_DURATION=15.0
EXCHANGE_INTERVAL=1000
INITIAL_TEMP=100000.0
COOLING_LAW="logarithmic"
ITERATIONS_PER_TEMP=5000
MAX_ITER_NO_IMPROVE=100000
MAX_ITER_GLOBAL=1000
THREAD_COUNTS=1

echo "Starting research with parameters:"
echo "Jobs: $JOBS, Processors: $PROCESSORS"
echo "Duration range: [$MIN_DURATION, $MAX_DURATION]"
echo "Cooling law: $COOLING_LAW"
echo "Testing thread counts: ${THREAD_COUNTS[*]}"
echo ""

for threads in "${THREAD_COUNTS[@]}"; do
    echo "=== Running with $threads threads ==="
    
    # Формируем имя файла для результатов
    TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
    OUTPUT_FILE="out/results_${threads}threads_${TIMESTAMP}.txt"
    
    # Запускаем алгоритм и сохраняем вывод
    ./AnnealingScheduler $JOBS $PROCESSORS $MIN_DURATION $MAX_DURATION \
                        $EXCHANGE_INTERVAL $INITIAL_TEMP $COOLING_LAW \
                        $ITERATIONS_PER_TEMP $MAX_ITER_NO_IMPROVE $MAX_ITER_GLOBAL \
                        $threads > "$OUTPUT_FILE" 2>&1
    
    # Проверяем успешность выполнения
    if [ $? -eq 0 ]; then
        echo "✓ Completed: $OUTPUT_FILE"
        
        # Извлекаем ключевые метрики для быстрого просмотра
        FITNESS=$(grep "Best solution fitness" "$OUTPUT_FILE" | tail -1 | awk '{print $4}')
        TIME_MS=$(grep "Algorithm completed" "$OUTPUT_FILE" | awk '{print $4}')
        IMPROVEMENT=$(grep "Improvement:" "$OUTPUT_FILE" | awk '{print $2}')
        
        echo "  Fitness: $FITNESS, Time: ${TIME_MS}ms, Improvement: $IMPROVEMENT"
    else
        echo "✗ Failed: $OUTPUT_FILE"
    fi
    
    echo ""
    
    # Пауза между запусками
    sleep 2
done

echo "Research completed! Results saved in 'out/' folder"

# Создаем сводный файл с результатами
SUMMARY_FILE="out/summary_${TIMESTAMP}.csv"
echo "Threads,Fitness,Time(ms),Improvement" > "$SUMMARY_FILE"

for threads in "${THREAD_COUNTS[@]}"; do
    LATEST_FILE=$(ls -t out/results_${threads}threads_*.txt | head -1)
    
    if [ -f "$LATEST_FILE" ]; then
        FITNESS=$(grep "Best solution fitness" "$LATEST_FILE" | tail -1 | awk '{print $4}' | sed 's/\.$//')
        TIME_MS=$(grep "Algorithm completed" "$LATEST_FILE" | awk '{print $4}')
        IMPROVEMENT=$(grep "Improvement:" "$LATEST_FILE" | awk '{print $2}')
        
        echo "$threads,$FITNESS,$TIME_MS,$IMPROVEMENT" >> "$SUMMARY_FILE"
    fi
done

echo "Summary saved: $SUMMARY_FILE"