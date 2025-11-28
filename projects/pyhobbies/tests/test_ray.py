import pytest
import pyhobbies # load the pybind library

def test_ray_components() -> None:
    from pyhobbies.raytrace import point, vector, ray
    p = point(1.0, 2.0, 3.0)
    d = vector(4.0, 5.0, 6.0)
    r = ray(p, d)
    pos = r.position()
    dir = r.direction()
    assert pos[0] == 1.0
    assert pos[1] == 2.0
    assert pos[2] == 3.0
    assert dir[0] == 4.0
    assert dir[1] == 5.0
    assert dir[2] == 6.0

def test_ray_distance_along() -> None:
    from pyhobbies.raytrace import point, vector, ray
    p = point(1.0, 2.0, 3.0)
    d = vector(1.0, 1.0, 1.0)
    r = ray(p, d)
    p2 = r.distance_along(1.0)
    assert p2[0] == 2.0
    assert p2[1] == 3.0
    assert p2[2] == 4.0

def test_ray_closest_point() -> None:
    from pyhobbies.raytrace import point, vector, ray
    p = point(0.0, 0.0, 0.0)
    d = vector(1.0, 0.0, 0.0)
    r = ray(p, d)
    p2 = point(0.0, 2.0, 0.0)
    # Finds the closest point on the ray to p2
    closest = r.closest(p2)
    assert closest[0] == 0.0
    assert closest[1] == 0.0
    assert closest[2] == 0.0
