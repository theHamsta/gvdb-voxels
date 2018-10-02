cmake_minimum_required(VERSION 2.8)
set(PROJNAME gImportVDB)

set( OPENVDB_LIBRARIES
 							tbb
							openvdb
							Half                                                                      
							IexMath                                                                  
							Iex                                                                             
							IlmThread                                                               
							Imath)

####################################################################################
# Compile PTX Files

find_package(CUDA REQUIRED)
#
file(GLOB CUDA_FILES RELATIVE "${BASE_DIRECTORY}" *.cu *.cuh)
message ( STATUS "Build CUDA kernels: ${CUDA_FILES}" )
# _COMPILEPTX ( SOURCES ${CUDA_FILES} TARGET_PATH ${EXECUTABLE_OUTPUT_PATH} GENERATED CUDA_PTX GENPATHS CUDA_PTX_PATHS INCLUDE "${CMAKE_CURRENT_SOURCE_DIR},${GVDB_INCLUDE_DIR}" OPTIONS -arch=compute_30 -code=sm_30 --ptxas-options=-v -O3 --use_fast_math --maxrregcount=128 )

#####################################################################################
# Asset Path
#
if ( NOT DEFINED ASSET_PATH ) 
  get_filename_component ( _assets "${CMAKE_CURRENT_SOURCE_DIR}/../shared_assets" REALPATH )
  set ( ASSET_PATH ${_assets} CACHE PATH "Full path to gvdb/shared_assets/" )  
endif()

include_directories(~/.local/include)


####################################################################################
# Executable
#
file(GLOB SOURCE_FILES *.cpp *.hpp *.inl *.h *.c)
# _LIST_ALL_SOURCE ( FILES ${SOURCE_FILES} ${COMMON_SOURCE_FILES} ${PACKAGE_SOURCE_FILES} ${UTIL_SOURCE_FILES} ${UTIL_OPTIX_FILES} ${UTIL_OPTIX_KERNELS} ${CUDA_FILES} ${PTX_SOURCES} )
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}")    
add_definitions(-DGVDB_IMPORTS -DGLEW_STATIC -DGLEW_NO_GLU)  
add_definitions(-DASSET_PATH="${ASSET_PATH}/")
cuda_add_executable (${PROJNAME} ${SOURCE_FILES} )
target_include_directories(${PROJNAME} PUBLIC /local/xu29mapu/projects/gvdb-voxels/source/gvdb_library/src/ ${CUDA_INCLUDE_DIRS} /local/xu29mapu/projects/OpenMesh/src/OpenMesh/Tools/Utils/)
set_property ( TARGET ${PROJNAME} APPEND PROPERTY DEPENDS ${PTX_SOURCES} )
# link_directories(/local/xu29mapu/projects/gvdb-voxels/source/gvdb_library/build/)
target_link_libraries(${PROJNAME} PUBLIC /local/xu29mapu/projects/gvdb-voxels/source/gvdb_library/build/libgvdb.so /local/xu29mapu/projects/cudpp/release/lib/libcudpp.a ${CUDA_LIBRARIES} ${OPENVDB_LIBRARIES} 
)

#####################################################################################
