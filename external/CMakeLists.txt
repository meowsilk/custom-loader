include(FetchContent)

set(FETCHCONTENT_QUIET ON)
set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_FOLDER "external")

FetchContent_Declare(
    freetype
    URL https://github.com/freetype/freetype/archive/refs/tags/VER-2-14-3.tar.gz
    URL_HASH SHA256=dc49de6b01a266eef4876a4dd34d9842c475d3e28ff2eff63bd2fb760ab56261
)
set(FT_DISABLE_ZLIB ON)
set(FT_DISABLE_BZIP2 ON)
set(FT_DISABLE_PNG ON)
set(FT_DISABLE_HARFBUZZ ON)
set(FT_DISABLE_BROTLI ON)

FetchContent_Declare(
    harfbuzz
    URL https://github.com/harfbuzz/harfbuzz/releases/download/14.5.1/harfbuzz-14.5.1.tar.xz
    URL_HASH SHA256=7e2fa4e8c7c98e8d8140671f5772542afaaa6acccfbd746506886b6d85f7f8d6
)
set(HB_HAVE_FREETYPE OFF)
set(HB_HAVE_GLIB OFF)
set(HB_HAVE_ICU OFF)
set(HB_HAVE_GDI OFF)
set(HB_HAVE_DIRECTWRITE OFF)
set(HB_BUILD_SUBSET OFF)
set(HB_BUILD_RASTER OFF)
set(HB_BUILD_VECTOR OFF)
set(HB_BUILD_GPU OFF)
set(HB_BUILD_UTILS OFF)

FetchContent_Declare(
    msdfgen
    URL https://github.com/Chlumsky/msdfgen/archive/refs/tags/v1.13.tar.gz
    URL_HASH SHA256=93cd1ad8918c1a78c5c96e82d4f4c77f0eb86c2e7e8579a0967e54196c4b7167
)
set(MSDFGEN_CORE_ONLY ON)
set(MSDFGEN_BUILD_STANDALONE OFF)
set(MSDFGEN_USE_VCPKG OFF)
set(MSDFGEN_USE_SKIA OFF)
set(MSDFGEN_INSTALL OFF)
set(MSDFGEN_DYNAMIC_RUNTIME OFF)

FetchContent_Declare(
    doctest
    URL https://github.com/doctest/doctest/archive/refs/tags/v2.5.3.tar.gz
    URL_HASH SHA256=174ebc4e769928959614789c5b4e9c3d0a0f81a62bb608756b127bfebfb21331
)
set(DOCTEST_NO_INSTALL ON)
set(DOCTEST_WITH_TESTS OFF)

FetchContent_MakeAvailable(freetype harfbuzz msdfgen)

if(LOADER_BUILD_TESTS)
    FetchContent_MakeAvailable(doctest)
endif()
