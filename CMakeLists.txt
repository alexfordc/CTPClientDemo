cmake_minimum_required(VERSION 3.5)
project(CTPClientDemo)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_VERBOSE_MAKEFILE ON)
find_package(Threads)
find_package(Boost 1.58 COMPONENTS log system thread filesystem)
ADD_DEFINITIONS(-DBOOST_ALL_DYN_LINK)
ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
#find_library(mysql NAMES libmysql PATHS ${MYSQL_DIR}/lib)


#add_subdirectory(ctpapi_linux64)



#set(SOURCE_FILES main.cpp MarketDataHandle.cpp MarketDataHandle.h TradingHandle.cpp TradingHandle.h DBDriver.cpp DBDriver.h)
set(SOURCE_FILES main.cpp MarketDataHandle.cpp MarketDataHandle.h TradingHandle.cpp TradingHandle.h DBDriver.cpp DBDriver.h Utility.cpp Utility.h)

#add_library(TCPClientDemo ${SOURCE_FILES})
#include_directories(${PROJECT_SOURCE_DIR})
#include_directories(${PROJECT_SOURCE_DIR}}/ctpapi_linux64)
#link_directories(${PROJECT_SOURCE_DIR}/ctpapi_linux64/)
#find_library(/home/biggreyhairboy/ClionProjects/CTPClientDemo/ctpapi_linux64 thostmduserapi.so )
add_library(thostmduserapi SHARED IMPORTED)
add_library(thosttraderapi SHARED IMPORTED)
SET_TARGET_PROPERTIES(thostmduserapi PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/ctpapi_linux64/thostmduserapi.so)
SET_TARGET_PROPERTIES(thosttraderapi PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/ctpapi_linux64/thosttraderapi.so)
add_executable(CTPClientDemo ${SOURCE_FILES})
target_link_libraries(CTPClientDemo mysqlcppconn thostmduserapi thosttraderapi ${CMAKE_THREAD_LIBS_INIT} ${Boost_LOG_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
