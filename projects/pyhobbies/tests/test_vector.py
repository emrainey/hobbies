import pytest
import pyhobbies # load the pybind library

def test_vector_components() -> None:
    from pyhobbies.raytrace import vector
    v = vector(1.0, 2.0, 3.0)
    assert v[0] == 1.0
    assert v[1] == 2.0
    assert v[2] == 3.0

def test_vector_scale() -> None:
    from pyhobbies.raytrace import vector
    v = vector(2.0, 4.0, 6.0)
    v *= 0.5
    assert v[0] == 1.0
    assert v[1] == 2.0
    assert v[2] == 3.0

def test_vector_divide() -> None:
    from pyhobbies.raytrace import vector
    v = vector(2.0, 4.0, 6.0)
    v /= 2.0
    assert v[0] == 1.0
    assert v[1] == 2.0
    assert v[2] == 3.0

def test_vector_accumulate() -> None:
    from pyhobbies.raytrace import vector
    v1 = vector(1.0, 2.0, 3.0)
    v2 = vector(4.0, 5.0, 6.0)
    v1 += v2
    assert v1[0] == 5.0
    assert v1[1] == 7.0
    assert v1[2] == 9.0

def test_vector_deaccumulate() -> None:
    from pyhobbies.raytrace import vector
    v1 = vector(4.0, 5.0, 6.0)
    v2 = vector(1.0, 2.0, 3.0)
    v1 -= v2
    assert v1[0] == 3.0
    assert v1[1] == 3.0
    assert v1[2] == 3.0

def test_vector_addition() -> None:
    from pyhobbies.raytrace import vector
    v1 = vector(1.0, 2.0, 3.0)
    v2 = vector(4.0, 5.0, 6.0)
    v3 = v1 + v2
    assert v3[0] == 5.0
    assert v3[1] == 7.0
    assert v3[2] == 9.0

def test_vector_subtraction() -> None:
    from pyhobbies.raytrace import vector
    v1 = vector(4.0, 5.0, 6.0)
    v2 = vector(1.0, 2.0, 3.0)
    v3 = v1 - v2
    assert v3[0] == 3.0
    assert v3[1] == 3.0
    assert v3[2] == 3.0

def test_vector_length() -> None:
    from pyhobbies.raytrace import vector
    v = vector(1.0, 2.0, 3.0)
    assert len(v) == 3

def test_vector_print() -> None:
    from pyhobbies.raytrace import vector
    v = vector(1.0, 2.0, 3.0)
    s = f"{v}"
    assert s is not None  # Just ensure it doesn't raise an error

def test_vector_magnitude() -> None:
    from pyhobbies.raytrace import vector
    v = vector(3.0, 4.0, 0.0)
    mag = v.magnitude()
    assert mag == 5.0

def test_vector_normalize() -> None:
    from pyhobbies.raytrace import vector
    v = vector(0.0, 3.0, 4.0)
    v.normalize()
    assert abs(v[0] - 0.0) < 1e-6
    assert abs(v[1] - 0.6) < 1e-6
    assert abs(v[2] - 0.8) < 1e-6

def test_vector_quadrance() -> None:
    from pyhobbies.raytrace import vector
    v = vector(1.0, 2.0, 2.0)
    quad = v.quadrance()
    assert quad == 9.0

def test_vector_dot() -> None:
    from pyhobbies.raytrace import vector, dot
    v1 = vector(1.0, 2.0, 3.0)
    v2 = vector(4.0, 5.0, 6.0)
    result = dot(v1, v2)
    assert result == 32.0

def test_vector_cross() -> None:
    from pyhobbies.raytrace import vector, cross
    v1 = vector(1.0, 0.0, 0.0)
    v2 = vector(0.0, 1.0, 0.0)
    result = cross(v1, v2)
    assert result[0] == 0.0
    assert result[1] == 0.0
    assert result[2] == 1.0

def test_vector_spread() -> None:
    from pyhobbies.raytrace import vector, spread
    v1 = vector(1.0, 0.0, 0.0)
    v2 = vector(0.0, 1.0, 0.0)
    # Computes the Rational Trigonometric spread (angle) between two vectors
    spread_value = spread(v1, v2)
    assert abs(spread_value - 1.0) < 1e-6  # 90 degrees in rational trigonometric terms