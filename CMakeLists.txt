cmake_minimum_required (VERSION 3.8)



project ("MyBrowser")

set(BOOST_ROOT "C:/boost") #Путь к папке Boost c папками include lib и share
set(OPENSSL_ROOT_DIR "C:/vcpkg/vcpkg/installed/x64-windows") #Путь к OpenSSL c папками include lib и share

set(PostgreSQL_ADDITIONAL_VERSIONS «17»)
set(PostgreSQL_ROOT "C:/Program Files/PostgreSQL/17")


find_package(OpenSSL REQUIRED)

find_package(Boost REQUIRED)


add_executable(MyBrowser MyBrowser.cpp crowler.cpp dataBase.cpp URLParser.cpp)

add_subdirectory("D:\\libraries\\libpqxx-7.10.0" libpqxx-build)   


target_compile_features(MyBrowser PRIVATE cxx_std_17)

target_include_directories(MyBrowser PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(MyBrowser PRIVATE OpenSSL::SSL OpenSSL::Crypto pqxx)
