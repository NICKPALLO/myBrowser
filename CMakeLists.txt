cmake_minimum_required (VERSION 3.8)

project ("MyBrowser")

set(BOOST_ROOT "C:/boost") #Путь к папке Boost c папками include lib и share
set(OPENSSL_ROOT_DIR "C:/vcpkg/vcpkg/installed/x64-windows") #Путь к OpenSSL c папками include lib и share

set(PostgreSQL_ADDITIONAL_VERSIONS "17")
set(PostgreSQL_ROOT "C:/Program Files/PostgreSQL/17")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)

find_package(Boost REQUIRED)

set(PROJECT_SOURCES
	MyBrowser.cpp
	crowler.cpp
	dataBase.cpp
	URLParser.cpp
	threadPool.cpp
	logger.cpp
	server.cpp
	server_certificate.hpp
	iniParser.h
)

add_executable(MyBrowser ${PROJECT_SOURCES})


add_subdirectory("D:\\libraries\\libpqxx-7.10.0" libpqxx-build)   


#target_compile_features(MyBrowser PRIVATE cxx_std_20)
#set_target_properties(MyBrowser PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

target_include_directories(MyBrowser PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(MyBrowser PRIVATE OpenSSL::SSL OpenSSL::Crypto pqxx)