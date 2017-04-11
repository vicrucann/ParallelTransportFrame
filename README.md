# Parallel Transport Frame algorithm implementation

A C++ and OpenSceneGraph-based class implementation based on *Parallel Transport Approach to Curve Framing* by A.J. Hanson and H. Ma, 1995. Some implementation details are based on Cinder's implementation. 

## Requirements

The project requires installed OpenSceneGraph (>= 3.4.0) and C++11 compatible compiler. 

## Usage of `PTFTube` in an external project

The `PTFTube` class is compiled into a separate library using CMake so that it can be easily used from an external project. If your project is hosted on github (or you use git), first, add `ParallelTransportFrame` as a submodule, e.g.: 

```
git submodule add https://github.com/vicrucann/ParallelTransportFrame.git your_path/src/third_party/ParallelTransportFrame
```

Second, edit the corresponding `CMakeLists.txt` file within your project to include the library:

```
add_subdirectory(ParallelTransportFrame/libPTFTube)
link_directories(${CMAKE_BINARY_DIR}/src/third_party/ParallelTransportFrame/libPTFTube)
# ...
target_link_libraries(libYours
    libPTFTube
    ${QT_LIBRARIES}
    ${OPENSCENEGRAPH_LIBRARIES}
    # ... any other libs
)
```

And now you can start using the `PTFTube` from within your code, e.g.: 

```
#include "libPTFTube/PTFTube.h"
// ...
PTFTube extrusion(path3d, 0.5, 5);
extrusion.build();

osg::Node* mesh = extrusion.generateTriMesh();
// ...
```

## Licence

See the corresponding [licence file](https://github.com/vicrucann/ParallelTransportFrame/blob/master/LICENSE).
