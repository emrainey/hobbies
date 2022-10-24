
# Geometry Library

A collection of primitives in a specific Real-space.
# What's in `geometry`?

## R2

These are the objects of 2D space

* Point
* Vector
* Line
* Ray

## R3

These are the objects of normal 3D space.

* Points
* Vectors
* Lines
* Rays
* Spheres
* Planes

## R4

Not sure I see a lot of point in doing this right now... as I'm not doing space time computations.

* Point - (Homogenized 3D points)
* Vectors
* TODO R4::Lines
* TODO R4::Quaternions

## Plan

* Use specialized `__m512d` where needed for 3/4 element storage.
* Use `__m128d` for 2D variants
* Once the geometry primitives are moved, and the libraries link again, then specialization for speed should be easier.