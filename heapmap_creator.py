import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

df = pd.read_csv('results/results_consistent.csv', index_col=0)

df = df.apply(pd.to_numeric, errors='coerce')

plt.figure(figsize=(16, 10))

sns.heatmap(df, 
            annot=True,  # Показывать значения в ячейках
            fmt='.0f',   # Формат чисел без десятичных знаков
            cmap='YlOrRd',  # Цветовая схема
            linewidths=0.5,
            linecolor='gray',
            cbar_kws={'label': 'Время выполнения (мс)'})

plt.title('Тепловая карта времени выполнения алгоритма\nВ зависимости от количества задач и процессоров', 
          fontsize=14, fontweight='bold', pad=20)
plt.xlabel('Количество задач', fontsize=12, fontweight='bold')
plt.ylabel('Количество процессоров', fontsize=12, fontweight='bold')

plt.xticks(rotation=45)
plt.yticks(rotation=0)

plt.tight_layout()
plt.savefig('results/heatmap.png', dpi=300, bbox_inches='tight')
