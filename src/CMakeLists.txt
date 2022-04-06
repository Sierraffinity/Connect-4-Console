file(
	GLOB_RECURSE SOURCE_FILES
	"./*.hpp"
	"./*.h"
	"./*.cpp"
	"./*.c"
)

add_executable(
    connect4
    ${SOURCE_FILES}
)