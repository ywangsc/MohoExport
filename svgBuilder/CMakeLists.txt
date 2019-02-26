set(SUBPROJECT_NAME svgBuilder)

set(SUBPROJECT_RESOURCES "")

set(SUBPROJECT_SOURCES
  ../lightningTestApp/HTTPDownloader.hpp
  ../lightningTestApp/HTTPDownloader.cpp
  main.cpp)

CreateProject("${SUBPROJECT_NAME}" "${SUBPROJECT_SOURCES}" "${SUBPROJECT_RESOURCES}")

hunter_add_package(CURL)
find_package(CURL CONFIG REQUIRED)
target_link_libraries(${SUBPROJECT_NAME} CURL::libcurl)
