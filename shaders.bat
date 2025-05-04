if not exist "bin/shaders" md "bin/shaders"

glslc assets/shaders/solid_skinned.vert -o bin/shaders/solid_skinned.vert
glslc assets/shaders/solid_skinned.frag -o bin/shaders/solid_skinned.frag