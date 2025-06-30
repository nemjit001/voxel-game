function(target_enable_extended_warnings TARGET_NAME)
	if (MSVC)
		target_compile_options(${TARGET_NAME} PRIVATE /W4)
	else() # Just assume clang/gcc
		target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)
	endif()
endfunction()

function(target_register_assets TARGET_NAME)
	set(REGISTERED_ASSETS "${ARGN}")
	foreach(ASSET_FILE IN LISTS REGISTERED_ASSETS)
		message(STATUS "Found asset file ${ASSET_FILE}")
		get_filename_component(INPUT_FILE_PATH ${ASSET_FILE} ABSOLUTE)
		set(OUTPUT_FILE_PATH "$<CONFIG>/${ASSET_FILE}")

		add_custom_command(OUTPUT ${OUTPUT_FILE_PATH}
			COMMAND ${CMAKE_COMMAND} -E copy ${INPUT_FILE_PATH} ${OUTPUT_FILE_PATH}
			COMMENT "[Assets] Copying ${ASSET_FILE} to binary dir"
			DEPENDS ${INPUT_FILE_PATH}
			VERBATIM
		)

		list(APPEND ASSET_OUTPUT_FILES ${OUTPUT_FILE_PATH})
	endforeach()

	add_custom_target(RegisterAssets DEPENDS ${ASSET_OUTPUT_FILES})
	add_dependencies(${TARGET_NAME} RegisterAssets)
endfunction()
