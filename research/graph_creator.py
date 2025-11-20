import pandas as pd
import matplotlib.pyplot as plt

with open('research/results_parallel.txt', 'r') as f:
    lines = f.readlines()

time_data = []
for i, line in enumerate(lines):
    if '|' in line and 'Потоки | Среднее время (мс)' not in line and '---------------------------' not in line:
        if i < 10:
            parts = line.split('|')
            threads = int(parts[0].strip())
            time_ms = int(parts[1].strip())
            time_data.append({'threads': threads, 'execution_time_ms': time_ms})
    if '|' not in line and len(time_data) > 0:
        break

improvement_data = []
for i, line in enumerate(lines):
    if '|' in line and 'Среднее улучшение' not in line and '---' not in line:
        if i > 10:
            parts = line.split('|')
            threads = int(parts[0].strip())
            improvement = float(parts[1].strip().replace('%', ''))
            improvement_data.append({'threads': threads, 'improvement_percentage': improvement})
    if '|' not in line and len(improvement_data) > 0:
        break

df_time = pd.DataFrame(time_data)
df_improvement = pd.DataFrame(improvement_data)

df = pd.merge(df_time, df_improvement, on='threads')

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 6))

ax1.plot(df['threads'], df['execution_time_ms'], marker='o', linewidth=2, markersize=8, color='blue')
ax1.set_xlabel('Количество потоков')
ax1.set_ylabel('Время выполнения (мс)')
ax1.set_title('Зависимость времени выполнения\nот количества потоков')
ax1.grid(True, alpha=0.3)
ax1.set_xticks(df['threads'])

ax2.plot(df['threads'], df['improvement_percentage'], marker='s', linewidth=2, markersize=8, color='red')
ax2.set_xlabel('Количество потоков')
ax2.set_ylabel('Процент улучшения (%)')
ax2.set_title('Зависимость процента улучшения\nот количества потоков')
ax2.grid(True, alpha=0.3)
ax2.set_xticks(df['threads'])

plt.tight_layout()
plt.savefig('research/parallel_performance.png', dpi=300, bbox_inches='tight')