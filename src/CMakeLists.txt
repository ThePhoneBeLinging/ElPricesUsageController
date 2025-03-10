cmake_minimum_required(VERSION 3.25)
project(ElPricesUsageController)

set(CMAKE_CXX_STANDARD 20)

find_package(fmt REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)
add_library(ElPricesUsageController STATIC
        ElPricesUsageController.cpp
        IUsageCollector.h
        MockUsageCollector.cpp
        MockUsageCollector.h
        PulseStorage.cpp
        PulseStorage.h
        UsageDay.cpp
        UsageCollector.cpp
        UsageCollector.h
)
target_include_directories(ElPricesUsageController PUBLIC include)
target_link_libraries(ElPricesUsageController PRIVATE Utility)
target_link_libraries(ElPricesUsageController PRIVATE SQLiteCpp)
target_link_libraries(ElPricesUsageController PRIVATE fmt::fmt)
target_link_libraries(ElPricesUsageController PRIVATE DatabaseAccessController)
target_include_directories(ElPricesUsageController PRIVATE ${GPIOD_INCLUDE_DIRS})
target_link_libraries(ElPricesUsageController PRIVATE ${GPIOD_LIBRARIES})
target_compile_options(ElPricesUsageController PRIVATE ${GPIOD_CFLAGS_OTHER})
