########################################################################
# Find the library for the Polymorphic Container
########################################################################

INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_PMC pmc)

FIND_PATH(
    PMC_INCLUDE_DIRS
    NAMES PMC/PMC.hpp
    HINTS $ENV{PMC_DIR}/include
        ${PC_PMC_INCLUDEDIR}
    PATHS /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    PMC_LIBRARIES
    NAMES pmc
    HINTS $ENV{PMC_DIR}/lib
        ${PC_PMC_LIBDIR}
    PATHS /usr/local/lib
          /usr/lib
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PMC DEFAULT_MSG PMC_LIBRARIES PMC_INCLUDE_DIRS)
MARK_AS_ADVANCED(PMC_LIBRARIES PMC_INCLUDE_DIRS)
