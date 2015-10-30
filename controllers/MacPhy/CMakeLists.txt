MESSAGE(STATUS "  Processing macphy.")

########################################################################
# Add includes and dependencies
########################################################################

########################################################################
# Build the library from source files
########################################################################
SET(sources
    MacPhyController.cpp
)

# Targets must be globally unique for cmake 
ADD_LIBRARY(controller_macphy SHARED ${sources})
SET_TARGET_PROPERTIES(controller_macphy PROPERTIES OUTPUT_NAME "macphy")
SET_TARGET_PROPERTIES(controller_macphy PROPERTIES DEFINE_SYMBOL "IRIS_CORE_DLL_EXPORTS")
IRIS_INSTALL(controller_macphy)
IRIS_APPEND_INSTALL_LIST("macphy")
