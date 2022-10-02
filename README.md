# SLIC (Simple Linear Iterative Clustering)
Fast and boundary followable method for computing Superpixels.

## Requirement
- CMake
- OpenCV 3.x

## How to build sample
```shell-session
mkdir sample/build
cd sample/build
cmake ../
make
```
3. Run.
```
./slic [image]
```
for example,
```
./slic ../../example/smoothie.png
```
![SLIC result](https://github.com/yuyuyu-bot/SLIC/blob/master/example/result.png)

### Reference
1. R. Achanta, A. Shaji, K. Smith, A. Lucchi, P. Fua, and S. Süsstrunk, "SLIC Superpixels", EPFL Technical Report no. 149300, June 2010.
