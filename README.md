# animorphs - deformable body simulator

This simulator uses Position-Based-Dynamics to simulate deformable agents.

The animorphs code compiles into a library and depends on https://github.com/ABRG-Models/morphologica (and thus all its dependencies)

## Dependencies

morphologica (clone this inside the animorphs tree), jsoncpp armadillo, opencv, lapack, opengl, glfw3.

## Compilation

```
git clone git@github.com:ABRG-Models/animorphs
cd animorphs
git clone git@github.com:ABRG-Models/morphologica
mkdir build
cd build
cmake ..
make
```

## Examples

See examples/ directory