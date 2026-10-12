file(GLOB_RECURSE asset_files CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/fonts/*" "${CMAKE_CURRENT_SOURCE_DIR}/images/*")

add_library(loader_assets OBJECT assets.rc)
set_source_files_properties(assets.rc PROPERTIES OBJECT_DEPENDS "${asset_files}")
