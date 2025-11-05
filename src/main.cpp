#include <iostream>
#include <memory>
#include <chrono>
#include "SimulatedAnnealing.h"
#include "ScheduleSolution.h"
#include "ScheduleMutation.h"
#include "SolutionGenerator.h"
#include "BoltzmannCooling.h"
#include "CauchyCooling.h"
#include "LogarithmicCooling.h"

int main() {
    std::cout << "=== Simulated Annealing Scheduler ===" << std::endl;
    
    // 1. Генерация тестовых данных
    std::cout << "\n1. Generating test data..." << std::endl;
    int jobCount = 8;
    int processorCount = 3;
    std::vector<double> jobDurations = {10.0, 15.0, 8.0, 12.0, 20.0, 5.0, 18.0, 9.0};
    
    std::cout << "   Jobs: " << jobCount << ", Processors: " << processorCount << std::endl;
    std::cout << "   Job durations: ";
    for (double duration : jobDurations) {
        std::cout << duration << " ";
    }
    std::cout << std::endl;
    
    // 2. Создание начального решения
    std::cout << "\n2. Creating initial solution..." << std::endl;
    SolutionGenerator generator;
    auto initialSolution = generator.generateRandomSolution(jobCount, processorCount, jobDurations);
    double initialFitness = initialSolution->evaluate();
    std::cout << "   Initial solution fitness: " << initialFitness << std::endl;
    
    // 3. Настройка алгоритма ИО
    std::cout << "\n3. Configuring simulated annealing..." << std::endl;
    auto mutation = std::make_shared<ScheduleMutation>();
    auto coolingLaw = std::make_shared<BoltzmannCooling>();
    
    SimulatedAnnealing sa;
    sa.setInitialSolution(initialSolution);
    sa.setMutation(mutation);
    sa.setCoolingLaw(coolingLaw);
    sa.setInitialTemperature(1000.0);
    sa.setIterationsPerTemperature(50);
    sa.setMaxIterationsWithoutImprovement(100);
    
    std::cout << "   Initial temperature: 1000.0" << std::endl;
    std::cout << "   Iterations per temperature: 50" << std::endl;
    std::cout << "   Max iterations without improvement: 100" << std::endl;
    
    // 4. Запуск и замер времени
    std::cout << "\n4. Running simulated annealing..." << std::endl;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    auto bestSolution = sa.run();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << "   Algorithm completed in " << duration.count() << " ms" << std::endl;
    
    // 5. Вывод результатов
    std::cout << "\n5. Results:" << std::endl;
    if (bestSolution) {
        auto scheduleSolution = std::dynamic_pointer_cast<ScheduleSolution>(bestSolution);
        if (scheduleSolution) {
            double bestFitness = scheduleSolution->evaluate();
            std::cout << "   Best solution fitness: " << bestFitness << std::endl;
            std::cout << "   Improvement: " << (initialFitness - bestFitness) << std::endl;
            
            // Вывод распределения по процессорам
            std::cout << "\n   Final schedule:" << std::endl;
            for (int j = 0; j < processorCount; ++j) {
                std::cout << "   Processor " << j << ": ";
                double processorTime = 0.0;
                for (int i = 0; i < jobCount; ++i) {
                    if (scheduleSolution->isJobAssignedToProcessor(i, j)) {
                        std::cout << "J" << i << "(" << jobDurations[i] << ") ";
                        processorTime += jobDurations[i];
                    }
                }
                std::cout << "→ Total: " << processorTime << std::endl;
            }
        }
    } else {
        std::cout << "   No solution found!" << std::endl;
    }
    
    // 6. Валидация решения
    std::cout << "\n6. Validating solution..." << std::endl;
    if (bestSolution) {
        auto scheduleSolution = std::dynamic_pointer_cast<ScheduleSolution>(bestSolution);
        if (scheduleSolution) {
            // Проверка что каждая работа назначена ровно на один процессор
            bool isValid = true;
            for (int i = 0; i < jobCount; ++i) {
                int assignmentCount = 0;
                for (int j = 0; j < processorCount; ++j) {
                    if (scheduleSolution->isJobAssignedToProcessor(i, j)) {
                        assignmentCount++;
                    }
                }
                if (assignmentCount != 1) {
                    isValid = false;
                    std::cout << "   ERROR: Job " << i << " assigned to " << assignmentCount << " processors!" << std::endl;
                }
            }
            
            if (isValid) {
                std::cout << "   ✓ Solution is valid - all jobs assigned correctly" << std::endl;
            } else {
                std::cout << "   ✗ Solution is invalid!" << std::endl;
            }
            
            // Проверка что все работы присутствуют
            std::vector<bool> jobsAssigned(jobCount, false);
            for (int i = 0; i < jobCount; ++i) {
                for (int j = 0; j < processorCount; ++j) {
                    if (scheduleSolution->isJobAssignedToProcessor(i, j)) {
                        jobsAssigned[i] = true;
                        break;
                    }
                }
            }
            
            bool allJobsAssigned = true;
            for (int i = 0; i < jobCount; ++i) {
                if (!jobsAssigned[i]) {
                    allJobsAssigned = false;
                    std::cout << "   ERROR: Job " << i << " is not assigned to any processor!" << std::endl;
                }
            }
            
            if (allJobsAssigned) {
                std::cout << "   ✓ All jobs are assigned to processors" << std::endl;
            } else {
                std::cout << "   ✗ Some jobs are missing!" << std::endl;
            }
        }
    }
    
    std::cout << "\n=== Algorithm finished ===" << std::endl;
    return 0;
}