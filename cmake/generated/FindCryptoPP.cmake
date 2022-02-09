# AUTOGENERATED, DON'T CHANGE THIS FILE!


if (TARGET CryptoPP)
  if (NOT CryptoPP_FIND_VERSION)
      set(CryptoPP_FOUND ON)
      return()
  endif()

  if (CryptoPP_VERSION)
      if (CryptoPP_FIND_VERSION VERSION_LESS_EQUAL CryptoPP_VERSION)
          set(CryptoPP_FOUND ON)
          return()
      else()
          message(FATAL_ERROR
              "Already using version ${CryptoPP_VERSION} "
              "of CryptoPP when version ${CryptoPP_FIND_VERSION} "
              "was requested."
          )
      endif()
  endif()
endif()

set(FULL_ERROR_MESSAGE "Could not find `CryptoPP` package.\n\tDebian: sudo apt update && sudo apt install libcrypto++-dev\n\tMacOS: brew install cryptopp")


include(FindPackageHandleStandardArgs)

find_library(CryptoPP_LIBRARIES_cryptlib_cryptopp
  NAMES cryptlib cryptopp
  PATH_SUFFIXES lib
)
list(APPEND CryptoPP_LIBRARIES ${CryptoPP_LIBRARIES_cryptlib_cryptopp})

find_path(CryptoPP_INCLUDE_DIRS_cryptopp_cryptlib_h
  NAMES cryptopp/cryptlib.h
  PATH_SUFFIXES include
)
list(APPEND CryptoPP_INCLUDE_DIRS ${CryptoPP_INCLUDE_DIRS_cryptopp_cryptlib_h})



if (CryptoPP_VERSION)
  set(CryptoPP_VERSION ${CryptoPP_VERSION})
endif()

if (CryptoPP_FIND_VERSION AND NOT CryptoPP_VERSION)
  include(DetectVersion)

  if (UNIX AND NOT APPLE)
    deb_version(CryptoPP_VERSION libcrypto++-dev)
  endif()
  if (APPLE)
    brew_version(CryptoPP_VERSION cryptopp)
  endif()
endif()

 
find_package_handle_standard_args(
  CryptoPP
    REQUIRED_VARS
      CryptoPP_LIBRARIES
      CryptoPP_INCLUDE_DIRS
      
    FAIL_MESSAGE
      "${FULL_ERROR_MESSAGE}"
)
mark_as_advanced(
  CryptoPP_LIBRARIES
  CryptoPP_INCLUDE_DIRS
  
)

if (NOT CryptoPP_FOUND)
  if (CryptoPP_FIND_REQUIRED)
      message(FATAL_ERROR "${FULL_ERROR_MESSAGE}. Required version is at least ${CryptoPP_FIND_VERSION}")
  endif()

  return()
endif()

if (CryptoPP_FIND_VERSION)
  if (CryptoPP_VERSION VERSION_LESS CryptoPP_FIND_VERSION)
      message(STATUS
          "Version of CryptoPP is '${CryptoPP_VERSION}'. "
          "Required version is at least '${CryptoPP_FIND_VERSION}'. "
          "Ignoring found CryptoPP."
      )
      set(CryptoPP_FOUND OFF)
      return()
  endif()
endif()

 
if (NOT TARGET CryptoPP)
  add_library(CryptoPP INTERFACE IMPORTED GLOBAL)

  target_include_directories(CryptoPP INTERFACE ${CryptoPP_INCLUDE_DIRS})
  target_link_libraries(CryptoPP INTERFACE ${CryptoPP_LIBRARIES})
  
  # Target CryptoPP is created
endif()

if (CryptoPP_VERSION)
  set(CryptoPP_VERSION "${CryptoPP_VERSION}" CACHE STRING "Version of the CryptoPP")
endif()