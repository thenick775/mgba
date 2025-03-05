set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
	-O3 \
	-s USE_SDL=2 \
	-s TOTAL_MEMORY=256mb \
	-s EXPORTED_RUNTIME_METHODS=[\"FS\",\"cwrap\",\"addFunction\",\"removeFunction\"] \
	-s RESERVED_FUNCTION_POINTERS=16 \
	-s ENVIRONMENT='web,worker' \
	-s PTHREAD_POOL_SIZE=3 \
	-s EXPORT_ES6=1 \
	-s MODULARIZE=1 \
	-s EXPORT_NAME=${CMAKE_PROJECT_NAME} \
	-s ELIMINATE_DUPLICATE_FUNCTIONS=1 \
	-s MIN_WEBGL_VERSION=2 \
	-s MAX_WEBGL_VERSION=2 \
	-gsource-map \
	--flto \
	-pthread \
	--pre-js ${CMAKE_CURRENT_SOURCE_DIR}/pre.js")
add_executable(${BINARY_NAME}.js main.c memory.c ../sdl/sdl-audio.c ../sdl/sdl-events.c)
set_target_properties(${BINARY_NAME}.js PROPERTIES
	COMPILE_DEFINITIONS "${OS_DEFINES};${FEATURE_DEFINES};${FUNCTION_DEFINES}"
	COMPILE_OPTIONS "-s;USE_SDL=2;-pthread"
	LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/pre.js"
	OUTPUT_NAME ${BINARY_NAME})
target_link_libraries(${BINARY_NAME}.js ${BINARY_NAME} ${OS_LIB})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mgba.d.ts ${CMAKE_CURRENT_BINARY_DIR}/${BINARY_NAME}.d.ts)
