import pytest
import pyhobbies # load the pybind library

def test_color_components() -> None:
    from pyhobbies.raytrace import color
    c = color(0.77, 0.55, 0.33)
    assert c.red() == 0.77
    assert c.green() == 0.55
    assert c.blue() == 0.33

def test_color_components() -> None:
    from pyhobbies.raytrace import color
    c = color(0.75, 0.5, 0.25)
    c.scale(0.5, False) # clamp?
    assert c.red() == 0.375
    assert c.green() == 0.25
    assert c.blue() == 0.125

def test_color_clamp() -> None:
    from pyhobbies.raytrace import color
    c = color(0.9, 1.1, 7.0)
    c.clamp()
    assert c.red() == 0.9
    assert c.green() == 1.0
    assert c.blue() == 1.0

def test_color_print() -> None:
    from pyhobbies.raytrace import color
    c = color(0.1, 0.2, 0.3)
    s = f"{c}"
    assert s == "<raytrace.color = R:0.100000 G:0.200000 B:0.300000>"