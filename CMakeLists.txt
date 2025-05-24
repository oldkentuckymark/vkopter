cmake_minimum_required(VERSION 3.22)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(vkopter LANGUAGES CXX)

#file(GLOB_RECURSE VKOPTER_HEADERS CONFIGURE_DEPENDS "*.h*")

#file(GLOB_RECURSE VKOPTER_INL CONFIGURE_DEPENDS "*.inl")

#file(GLOB_RECURSE VKOPTER_SOURCES CONFIGURE_DEPENDS "*.cpp")

file(GLOB_RECURSE VKOPTER_DATA_FILES CONFIGURE_DEPENDS "data/*")

#file(GLOB_RECURSE CITYGEN_SCRIPTS CONFIGURE_DEPENDS "data/citygen/scripts/*")

file(GLOB_RECURSE JOLT_HEADERS CONFIGURE_DEPENDS "src/Jolt/*.h")

file(GLOB_RECURSE JOLT_SOURCES CONFIGURE_DEPENDS "src/Jolt/*.cpp")

set(LICENCES
	src/game/entt/LICENSE
	src/Jolt/LICENSE
	LICENSE
)



set (CITYTEST_IMAGES
	data/citygen/img/00.bmp
	data/citygen/img/01.bmp
	data/citygen/img/02.bmp
	data/citygen/img/03.bmp
	data/citygen/img/04.bmp
	data/citygen/img/05.bmp
	data/citygen/img/06.bmp
	data/citygen/img/07.bmp
	data/citygen/img/08.bmp
	data/citygen/img/09.bmp
	data/citygen/img/10.bmp
	data/citygen/img/11.bmp
	data/citygen/img/12.bmp

        data/textures/texture.png

)

set (CONFIG_FILES
	data/config/config.json

)

set (HEIGHTMAP_IMAGES
	data/maps/test/hm.bmp

)

set (MESH_FILES
        data/meshes/untitled.gltf

	data/meshes/terrain-00.gltf
	data/meshes/terrain-01.gltf
	data/meshes/terrain-02.gltf
	data/meshes/terrain-03.gltf
	data/meshes/terrain-04.gltf
	data/meshes/terrain-05.gltf
	data/meshes/terrain-06.gltf
	data/meshes/terrain-07.gltf
	data/meshes/terrain-08.gltf
	data/meshes/terrain-09.gltf
	data/meshes/terrain-10.gltf
	data/meshes/terrain-11.gltf
	data/meshes/terrain-12.gltf
	data/meshes/terrain-13.gltf

        data/meshes/road-00.gltf
        data/meshes/road-01.gltf
        data/meshes/road-02.gltf
        data/meshes/road-03.gltf
        data/meshes/road-04.gltf
        data/meshes/road-05.gltf
        data/meshes/road-06.gltf
        data/meshes/road-07.gltf
        data/meshes/road-08.gltf
        data/meshes/road-09.gltf
        data/meshes/road-10.gltf
        data/meshes/road-11.gltf
        data/meshes/road-12.gltf
        data/meshes/road-13.gltf

)

set(SHADER_SOURCES
	data/shaders/phong/phong.frag
	data/shaders/phong/phong.vert

	data/shaders/terrain/terrain.frag
	data/shaders/terrain/terrain.vert

	data/shaders/water/water.tcs
	data/shaders/water/water.tes
	data/shaders/water/water.vert
	data/shaders/water/water.frag
)


set(VKOPTER_SOURCES
	src/vkopter.cpp
)

set(VKOPTER_HEADERS
	src/game/citygen/atom.hpp
	src/game/citygen/atomupdater.hpp
	src/game/citygen/eventwindow.hpp
	src/game/citygen/grid.hpp
	src/game/camera.hpp
	src/game/terrain.hpp
	src/game/simulation.hpp
	src/game/entity.hpp

	src/render/mesh.hpp
	src/render/light.hpp
	src/render/material.hpp
	src/render/memorymanager.hpp
	src/render/renderobject.hpp
	src/render/vk_mem_alloc.h
	src/render/vulkanrenderer.hpp

	src/settings.hpp

	src/util/array2d.hpp
	src/util/fixed_vector.hpp
	src/util/json.hpp
	src/util/read_file.hpp
	src/util/stb_image.h
	src/util/stb_image_write.h
	src/util/tiny_gltf.hpp

	src/vulkanwindow.hpp
)

set(CITYTEST_SOURCES
	src/citytest.cpp
)

set(ENTT_HEADERS
	src/game/entt/entt.hpp
)


add_executable(vkopter
	${ENTT_HEADERS}
	${JOLT_HEADERS}
	${JOLT_SOURCES}
	${VKOPTER_HEADERS}
	${VKOPTER_SOURCES}

	)
add_executable(citytest
	${CITYTEST_SOURCES}
	${CITYTEST_HEADERS}
	)

if(WIN32)
	find_library(SDL2MAIN_LIBRARY NAMES SDL2main PATHS "$ENV{VULKAN_SDK}/Lib")
	find_library(SDL2_LIBRARY NAMES SDL2 PATHS "$ENV{VULKAN_SDK}/Lib" )
	find_library(VULKAN_LIBRARY NAMES vulkan-1 PATHS "$ENV{VULKAN_SDK}/Lib")
		find_library(ATOMIC_LIBRARY atomic)
		#set_source_files_properties(src/vkopter.cpp PROPERTIES COMPILE_FLAGS /bigobj)

	target_include_directories(vkopter PUBLIC $ENV{VULKAN_SDK}/Include)
	target_include_directories(citytest PUBLIC $ENV{VULKAN_SDK}/Include)

		if(MSVC)
			target_compile_options(vkopter PUBLIC /bigobj)
			target_compile_options(citytest PUBLIC /bigobj)
		endif()

		if(MINGW)
		   #add_compile_options("-Wa,-mbig-obj")
		   #target_compile_options(citytest PRIVATE "-Wa,-mbig-obj")
		   #target_compile_options(citytest PUBLIC "-Og")
		   #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj O2")
		   #set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -mbig-obj")
		endif()
endif()




if(UNIX)
	find_library(SDL2MAIN_LIBRARY SDL2main)
	find_library(SDL2_LIBRARY SDL2)
		find_library(VULKAN_LIBRARY vulkan)
		find_library(ATOMIC_LIBRARY NAMES libatomic.so.1)
		find_library(PTHREADS_LIBRARY NAMES pthread)
endif()



target_compile_features(vkopter PUBLIC cxx_std_20)
set_target_properties(vkopter PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(vkopter ${SDL2MAIN_LIBRARY} ${SDL2_LIBRARY} ${VULKAN_LIBRARY} ${PTHREAD_LIBRARY})
target_include_directories(vkopter PUBLIC src)



target_compile_features(citytest PUBLIC cxx_std_20)
set_target_properties(citytest PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(citytest ${SDL2MAIN_LIBRARY} ${SDL2_LIBRARY} ${VULKAN_LIBRARY})
target_include_directories(citytest PUBLIC src)





add_custom_target(data SOURCES ${VKOPTER_DATA_FILES})

#add_custom_target(scripts SOURCES ${CITYGEN_SCRIPTS})







