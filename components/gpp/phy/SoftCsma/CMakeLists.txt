MESSAGE(STATUS "  Processing softcsma.")

########################################################################
# Add includes and dependencies
########################################################################

########################################################################
# Build the library from source files
########################################################################
SET(sources
    SoftCsmaComponent.cpp
)

ADD_LIBRARY(comp_gpp_phy_softcsma SHARED ${sources})

SET_TARGET_PROPERTIES(comp_gpp_phy_softcsma PROPERTIES OUTPUT_NAME "softcsma")
IRIS_INSTALL(comp_gpp_phy_softcsma)
IRIS_APPEND_INSTALL_LIST(softcsma)
