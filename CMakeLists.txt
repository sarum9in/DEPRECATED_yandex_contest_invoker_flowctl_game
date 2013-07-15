project(yandex_contest_invoker_flowctl_game)

include(${CMAKE_SOURCE_DIR}/system-config.cmake OPTIONAL)

cmake_minimum_required(VERSION 2.8)

include(BunsanCommon)

bunsan_use_boost(system filesystem serialization program_options)

bunsan_use_bunsan(common)
bunsan_use(
    yandex_contest_common
    yandex_contest_system
    yandex_contest_invoker
)

bunsan_add_shared_library(${PROJECT_NAME}
    src/lib/Killer.cpp
    src/lib/KillerImpl.cpp
    src/lib/KillerInterface.cpp
    src/lib/KillerStreamInterface.cpp

    src/lib/Broker.cpp
    src/lib/BrokerImpl.cpp
    src/lib/BrokerImplCommunicate.cpp
    src/lib/BrokerInterface.cpp
    src/lib/BrokerTextInterface.cpp

    src/lib/Tokenizer.cpp
    src/lib/SharedTokenizer.cpp

    src/lib/Configurator.cpp
)

set(tokenizers)

macro(add_tokenizer target src)
    add_library(${target} SHARED ${src})
    list(APPEND tokenizers ${target})
endmacro()

add_tokenizer(${PROJECT_NAME}_tokenizer_split_char src/tokenizers/split_char.c)
add_tokenizer(${PROJECT_NAME}_tokenizer_split src/tokenizers/split.c)

yandex_contest_add_binary_targets(bin_targets killer broker)

bunsan_install_headers()
bunsan_install_targets(${PROJECT_NAME} ${bin_targets} ${tokenizers})

bunsan_include_tests()
