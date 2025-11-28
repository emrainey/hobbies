import pytest
import pyhobbies # load the pybind library

def test_point_components() -> None:
    from pyhobbies.raytrace import point
    p = point(1.0, 2.0, 3.0)
    assert p[0] == 1.0
    assert p[1] == 2.0
    assert p[2] == 3.0

def test_point_scale() -> None:
    from pyhobbies.raytrace import point
    p = point(2.0, 4.0, 6.0)
    p *= 0.5
    assert p[0] == 1.0
    assert p[1] == 2.0
    assert p[2] == 3.0

def test_point_zero() -> None:
    from pyhobbies.raytrace import point
    p = point(5.0, 10.0, 15.0)
    p.zero()
    assert p[0] == 0.0
    assert p[1] == 0.0
    assert p[2] == 0.0

def test_point_setitem() -> None:
    from pyhobbies.raytrace import point
    p = point(0.0, 0.0, 0.0)
    p[0] = 5.0
    p[1] = 10.0
    p[2] = 15.0
    assert p[0] == 5.0
    assert p[1] == 10.0
    assert p[2] == 15.0

def test_point_length() -> None:
    from pyhobbies.raytrace import point
    p = point(1.0, 2.0, 3.0)
    assert len(p) == 3

def test_point_print() -> None:
    from pyhobbies.raytrace import point
    p = point(1.0, 2.0, 3.0)
    s = f"{p}"
    assert s is not None  # Just ensure it doesn't raise an error