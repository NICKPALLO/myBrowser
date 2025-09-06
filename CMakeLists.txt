cmake_minimum_required (VERSION 3.8)

project ("MyBrowser")

set(BOOST_ROOT "C:/boost") #Путь к папке Boost c папками include lib и share
set(OPENSSL_ROOT_DIR "C:/vcpkg/vcpkg/installed/x64-windows") #Путь к OpenSSL c папками include lib и share

set(PostgreSQL_ADDITIONAL_VERSIONS "17")
set(PostgreSQL_ROOT "C:/Program Files/PostgreSQL/17")

#необходим 20 стандарт
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Для исполняемых файлов (.exe, .out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


find_package(OpenSSL REQUIRED)

find_package(Boost REQUIRED)

set(PROJECT_SOURCES
	src/main.cpp
	src/MyBrowser.cpp
	src/crowler.cpp
	src/dataBase.cpp
	src/URLParser.cpp
	src/threadPool.cpp
	src/logger.cpp
	src/server.cpp
)

add_executable(MyBrowser ${PROJECT_SOURCES})

add_subdirectory("D:\\libraries\\libpqxx-7.10.0" libpqxx-build)   

target_include_directories(MyBrowser PRIVATE ${Boost_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(MyBrowser PRIVATE OpenSSL::SSL OpenSSL::Crypto pqxx)