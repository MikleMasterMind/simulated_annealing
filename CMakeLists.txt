cmake_minimum_required(VERSION 3.10)
project(SimulatedAnnealingScheduler)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Отключение построения в подпапке build
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Список исходных файлов
set(SOURCES
    src/main.cpp
    src/ISolution.cpp
    src/IMutation.cpp
    src/ICoolingLaw.cpp
    src/BoltzmannCooling.cpp
    src/CauchyCooling.cpp
    src/LogarithmicCooling.cpp
    src/SimulatedAnnealing.cpp
    src/ScheduleSolution.cpp
    src/ScheduleMutation.cpp
    src/SolutionGenerator.cpp
    src/IDataGenerator.cpp
    src/CSVDataGenerator.cpp
    src/IDataReader.cpp
    src/CSVDataReader.cpp
)

# Список заголовочных файлов
set(HEADERS
    src/ISolution.h
    src/IMutation.h
    src/ICoolingLaw.h
    src/BoltzmannCooling.h
    src/CauchyCooling.h
    src/LogarithmicCooling.h
    src/SimulatedAnnealing.h
    src/ScheduleSolution.h
    src/ScheduleMutation.h
    src/SolutionGenerator.h
    src/IDataGenerator.h
    src/CSVDataGenerator.h
    src/IDataReader.h
    src/CSVDataReader.h
)

# Создание исполняемой программы
add_executable(AnnealingScheduler ${SOURCES} ${HEADERS})

# Настройка свойств компиляции
target_compile_features(AnnealingScheduler PRIVATE cxx_std_17)

# Настройка включения заголовков
target_include_directories(AnnealingScheduler PRIVATE src)

# Настройка для отладки/релиза
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(AnnealingScheduler PRIVATE DEBUG)
    target_compile_options(AnnealingScheduler PRIVATE -g -O0)
else()
    target_compile_options(AnnealingScheduler PRIVATE -O2)
endif()

# Предупреждения компилятора
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(AnnealingScheduler PRIVATE 
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wno-unused-parameter
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(AnnealingScheduler PRIVATE 
        /W4
        /WX
    )
endif()