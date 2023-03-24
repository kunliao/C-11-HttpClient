cmake_minimum_required(VERSION 3.10)
project(HttpClient)

set(CMAKE_CXX_STANDARD 14)


include_directories(include)

add_executable(HttpClient main.cpp HttpClient.cpp HttpResponse.cpp HttpCookie.cpp Thread.cpp)

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    add_library(curl STATIC IMPORTED)
    add_library(zlib STATIC IMPORTED)
    add_library(ssl STATIC IMPORTED)
    add_library(crypto STATIC IMPORTED)
    set_target_properties(curl PROPERTIES  IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/lib/linux/libcurl.a)
    set_target_properties(zlib PROPERTIES  IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/lib/linux/libz.a)
    set_target_properties(ssl PROPERTIES  IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/lib/linux/libssl.a)
    set_target_properties(crypto PROPERTIES  IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/lib/linux/libcrypto.a)
    target_link_libraries(HttpClient curl zlib ssl crypto pthread dl)
elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    if(CMAKE_CL_64)                                                                              
       
    else(CMAKE_CL_64)
        
    endif(CMAKE_CL_64)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(HttpClient curl) #mac自带curl
else()

endif()
