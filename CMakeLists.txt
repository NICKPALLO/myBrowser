cmake_minimum_required (VERSION 3.8)


project ("MyBrowser")

set(BOOST_ROOT "C:/boost") #Путь к папке Boost c папками include lib и share
set(OPENSSL_ROOT_DIR "C:/vcpkg/vcpkg/installed/x64-windows") #Путь к OpenSSL c папками include lib и share

find_package(OpenSSL REQUIRED)

find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(MyBrowser MyBrowser.cpp crowler.cpp)

target_link_libraries(MyBrowser PRIVATE OpenSSL::SSL OpenSSL::Crypto)

#target_link_libraries(MyBrowser PRIVATE Boost::asio)
#target_include_directories(MyBrowser PRIVATE ${Boost_INCLUDE_DIRS})

#target_include_directories(MyBrowser PRIVATE ${Boost_INCLUDE_DIRS})
#target_link_libraries(MyBrowser PRIVATE Boost::system Boost::date_time Boost::context)

#target_link_libraries(MyBrowser Boost::asio)