# All #includes in test programs have to be #include <morph/header.h>
include_directories(BEFORE ${PROJECT_SOURCE_DIR})

# Colliding softmats
add_subdirectory(softcollision)

# Bags and contact already
add_subdirectory(bags)

# Huddling
add_subdirectory(huddling)

# Standalone
add_subdirectory(standalone)
