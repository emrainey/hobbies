import pyhobbies
from pyhobbies.raytrace import point, sphere, scene

def test_scene_simple_sphere -> None:
    scene = scene()
    scene.from(point(7,8,9))
    scene.at(point(6,7,8))
    scene.add(sphere(point(1,2,3), 5))
    scene.render()
    scene.save("file.tga")
