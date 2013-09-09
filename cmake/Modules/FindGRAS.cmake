########################################################################
# Find the library for the GNU Radio Advanced Scheduler
########################################################################

INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_GRAS gras)

FIND_PATH(
    GRAS_INCLUDE_DIRS
    NAMES gras/gras.hpp
    HINTS $ENV{GRAS_DIR}/include
        ${PC_GRAS_INCLUDEDIR}
    PATHS /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    GRAS_LIBRARIES
    NAMES gras
    HINTS $ENV{GRAS_DIR}/lib
        ${PC_GRAS_LIBDIR}
    PATHS /usr/local/lib
          /usr/lib
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GRAS DEFAULT_MSG GRAS_LIBRARIES GRAS_INCLUDE_DIRS)
MARK_AS_ADVANCED(GRAS_LIBRARIES GRAS_INCLUDE_DIRS)
