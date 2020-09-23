bl_info = {
    "name": "VBO exporter",
    "author": "IceCubeGames",
    "version": (0, 1),
    "blender": (2, 75, 0),
    "location": "File > Export",
    "description": "VBO exporter",
    "warning": "",
    "wiki_url": "",
    "category": "Export",
}

import bpy
import json
from os import path
import struct
from mathutils import Vector, Euler
from math import sqrt, ceil, radians
import subprocess
import sys

def write_gi_block(context, filepath):
    directory, _ = path.split(filepath)

    object = bpy.context.active_object
    name = object.name
    vertices = path.join(directory, name + "_vertices.vbo")
    faces = path.join(directory, name + "_faces.vbo")

    with open(vertices, "wb") as vertex_file, open(faces, "wb") as faces_file:
        matrix = object.matrix_world
        
        mesh = object.to_mesh()
        mesh.calc_loop_triangles()
        
        uv_layer = mesh.uv_layers["UVMap"]
        vertex_size = 0
        
        for triangle in mesh.loop_triangles:
            for vertex_index in triangle.vertices:
                faces_file.write(struct.pack('I', vertex_size))
                
                vertex_file.write(
                    struct.pack(
                        'fff', 
                        *(matrix @ mesh.vertices[vertex_index].co)
                    ) + 
                    struct.pack(
                        'fff', *(matrix @ Vector(
                            mesh.vertices[vertex_index].normal[:] + (0,)
                        ))[:3]
                    ) +
                    struct.pack('ff', 0, 0)
                )
                
                vertex_size += 1
                

    return {'FINISHED'}

# ExportHelper is a helper class, defines filename and
# invoke() function which calls the file selector.
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty
from bpy.types import Operator
   
class ExportVBOObject(Operator, ExportHelper):
    """Export to Gi Block file"""
    # important since its how bpy.ops.import_test.some_data is constructed
    bl_idname = "vbo.object_export"  
    bl_label = "Export VBO Object"

    # ExportHelper mixin class uses this
    filename_ext = ".vbo"

    filter_glob = StringProperty(
        default="*.vbo",
        options={'HIDDEN'},
        maxlen=255,  # Max internal buffer length, longer would be clamped.
    )

    def execute(self, context):
        return write_gi_block(context, self.filepath)

    def menu_func_export(self, context):
        self.layout.operator(bl_idname, text="Object VBO (.vbo)")

def register():
    bpy.utils.register_class(ExportBrechpunktObject)
    bpy.types.INFO_MT_file_export.append(ExportBrechpunktObject.menu_func_export)

def unregister():
    unregisterSceneProperties()
    bpy.utils.unregister_class(ExportBrechpunktObject)
    bpy.types.INFO_MT_file_export.remove(ExportBrechpunktObject.menu_func_export)


if __name__ == "__main__":
    write_gi_block(bpy.context, bpy.data.filepath)
