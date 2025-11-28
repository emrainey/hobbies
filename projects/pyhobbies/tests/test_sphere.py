import pytest
import pyhobbies # load the pybind library

def test_point_components() -> None:
    from pyhobbies.raytrace import sphere, point, medium
    p = point(1.0, 2.0, 3.0)
    s = sphere(p, 5.0)
    assert s.position()[0] == 1.0
    assert s.position()[1] == 2.0
    assert s.position()[2] == 3.0

def test_sphere_move() -> None:
    from pyhobbies.raytrace import sphere, point, vector
    p = point(1.0, 2.0, 3.0)
    s = sphere(p, 5.0)
    move_vec = vector(2.0, 3.0, 4.0)
    s.move_by(move_vec)
    assert s.position()[0] == 3.0
    assert s.position()[1] == 5.0
    assert s.position()[2] == 7.0

@pytest.mark.skip(reason="material binding not yet implemented")
def test_sphere_material() -> None:
    from pyhobbies.raytrace import sphere, point, medium
    p = point(1.0, 2.0, 3.0)
    s = sphere(p, 5.0)
    m = medium()
    s.set_material(m)
    m2 = s.get_material()
    assert m2 == m