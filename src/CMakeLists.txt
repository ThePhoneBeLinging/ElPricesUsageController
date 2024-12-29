cmake_minimum_required(VERSION 3.30)
project(ElPricesUsageController)

set(CMAKE_CXX_STANDARD 20)


add_library(ElPricesUsageController STATIC
        ElPricesUsageController.cpp
        IUsageCollector.h
        MockUsageCollector.cpp
        MockUsageCollector.h
        PulseStorage.cpp
        PulseStorage.h
)
target_include_directories(ElPricesUsageController PUBLIC include)
target_link_libraries(ElPricesUsageController PRIVATE Utility)
target_link_libraries(ElPricesUsageController PRIVATE SQLiteCpp)