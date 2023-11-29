project(PK_B210 DESCRIPTION "main for PK")
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS system REQUIRED
                              filesystem REQUIRED
                              program_options REQUIRED
                              log REQUIRED)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -ffast-math -std=c++14 -O3")

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)

add_executable(PK_B210 main.cpp
                       RecorderMagister.cpp
                       FileOut.cpp
                       config/RecorderConfig.cpp
                       config/Config.cpp)           

target_include_directories(PK_B210 PRIVATE ${KITTY_INSTALL_PATH}/include
                                            PRIVATE ${KITTY_INSTALL_PATH}/include
                                            PRIVATE ${KITTY_INSTALL_PATH}/rfnoc/include
                                            PRIVATE .)

target_link_libraries(PK_B210 Threads::Threads
                              Boost::program_options
                              Boost::system
                              Boost::filesystem
                              Boost::log
                              stdc++fs
                              ${KITTY_INSTALL_PATH}/rfnoc/lib/libuhd.so)

install(TARGETS PK_B210 DESTINATION ${KITTY_INSTALL_PATH}/bin)
install(DIRECTORY DESTINATION ${KITTY_INSTALL_PATH}/bin/input)
install(DIRECTORY DESTINATION ${KITTY_INSTALL_PATH}/bin/output)
