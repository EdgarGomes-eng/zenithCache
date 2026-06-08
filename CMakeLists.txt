cmake_minimum_required(VERSION 3.22)
project(ZenithCache LANGUAGES CXX)

# Enforçar o C++20 de forma estrita
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Flags de Compilação de Nível de Produção (Garante código limpo e seguro)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(
        -Wall          # Ativa avisos comuns
        -Wextra        # Ativa avisos adicionais importantes
        -Werror        # Transforma todos os avisos em erros (não aceitamos código desleixado)
        -Wshadow       # Avisa se uma variável local esconder outra variável
        -Wpedantic     # Garante conformidade estrita com o padrão ISO C++
        -O3            # Otimização máxima de performance para o binário
    )
endif()

# Encontrar o pacote do sistema para Threads (essencial para std::jthread)
find_package(Threads REQUIRED)

# Listar os ficheiros de implementação de forma explícita (Boa prática de Modern CMake)
set(SOURCE_FILES
    src/main.cpp
    src/EventLoop.cpp
    src/Server.cpp
    src/Connection.cpp
)

# Definir o nosso binário final
add_executable(zenithcache ${SOURCE_FILES})

# Configurar caminhos de inclusão estruturados
target_include_directories(zenithcache PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Vincular as bibliotecas necessárias (neste caso, suporte a threads nativas)
target_link_libraries(zenithcache PRIVATE Threads::Threads)