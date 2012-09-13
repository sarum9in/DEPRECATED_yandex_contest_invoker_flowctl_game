project(yandex_contest_invoker_flowctl_game)

include(${CMAKE_SOURCE_DIR}/system-config.cmake OPTIONAL)

cmake_minimum_required(VERSION 2.8)

include(YandexContestCommon)

# BOOST
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS system filesystem serialization unit_test_framework program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
list(APPEND libraries ${Boost_LIBRARIES})

# LibEvent
#find_package(PkgConfig)
#pkg_check_modules(REQUIRED libevent)
#link_directories(${LibEvent_LIBRARY_DIRS})
#include_directories(${LibEvent_INCLUDE_DIRS})
#list(APPEND libraries ${LibEvent_LIBRARIES})

# yandex_contest_*
list(APPEND libraries
    yandex_contest_common
    yandex_contest_system
)

# Local
include_directories(include)

# Targets

add_library(${PROJECT_NAME} SHARED
    src/lib/Killer.cpp
    src/lib/KillerImpl.cpp
    src/lib/KillerInterface.cpp
    src/lib/KillerStreamInterface.cpp

    src/lib/Broker.cpp
    src/lib/BrokerInterface.cpp
    src/lib/BrokerTextInterface.cpp

    src/lib/Tokenizer.cpp
    src/lib/SharedTokenizer.cpp
)
target_link_libraries(${PROJECT_NAME} ${libraries})

add_library(${PROJECT_NAME}_tokenizer_split_char SHARED
    src/tokenizers/split_char.c
)

add_library(${PROJECT_NAME}_tokenizer_split SHARED
    src/tokenizers/split.c
)

add_binary_targets(bin_targets killer broker)

# installation
install(DIRECTORY include DESTINATION .)
install(TARGETS ${PROJECT_NAME} ${bin_targets}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

yandex_contest_tests()
