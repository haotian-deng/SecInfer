add_executable(test_alexnet_oR alexnet_main.cpp common/alexnet.cpp)

if (NOT MOTION_BUILD_BOOST_FROM_SOURCES)
    find_package(Boost
            COMPONENTS
            program_options 
            REQUIRED)
endif ()

find_package(OpenMP REQUIRED)

if(OpenMP_CXX_FOUND)
    target_link_libraries(test_alexnet_oR PUBLIC OpenMP::OpenMP_CXX)
    target_compile_definitions(test_alexnet_oR PRIVATE PARALLEL)
endif()

target_link_libraries(test_alexnet_oR PUBLIC
    MOTION::motion
    Boost::program_options
    # 使用实际的大写库名称
    OPENFHEpke
    OPENFHEcore
    OPENFHEbinfhe  # 如果需要
    # 其他库
)


# include_directories(${PROJECT_SOURCE_DIR}/extern/ScalableMixedModeMPC/emp-aby/converter)
include_directories(${PROJECT_SOURCE_DIR}/extern/ScalableMixedModeMPC/)
