# SLIC (Simple Linear Iterative Clustering)
Fast and boundary followable method for computing Superpixels.

## Reqirement
OpenCV 3.x<br>
CMake 3.1 or later

## How to use
Clone this repository and move to directory "SLIC".
```
mkdir build
cd build
cmake ../
make
```
and execute with below statement
```
./slic_test.exe [image]
```
for example
```
./slic_test.exe ../example/vegetables.png
```

### Reference
1. R. Achanta, A. Shaji, K. Smith, A. Lucchi, P. Fua, and S. Süsstrunk, "SLIC Superpixels", EPFL Technical Report no. 149300, June 2010.
