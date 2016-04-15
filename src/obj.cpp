/*
 * Copyright 2014 Nicola Gigante
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "octree.h"
#include "glm.h"

#include <vector>
#include <tuple>
#include <iterator>

namespace ocmesh {
namespace details {
    
    // I don't use glm vectors here because they're not constexpr,
    // but we don't need to do anything with them anyway, other
    // than printing, so it's not a problem.
    // Note that these are returned in the same order as the
    // voxel::direction enum
    constexpr std::array<std::array<int, 3>, 6> make_normals() {
        return {
            -1,  0,  0, // Left
             1,  0,  0, // Right
             0, -1,  0, // Down
             0,  1,  0, // Up
             0,  0, -1, // Back
             0,  0,  1  // Front
        };
    }
    
    struct face {
        size_t normal;
        std::array<size_t, 6> vertices;
    };
    
    constexpr std::array<face, 6> make_faces() {
        return {
            // Left face:   First triangle
            voxel::left,    voxel::right_bottom_front,
                            voxel::right_top_front   ,
                            voxel::left_top_front    ,

            //              Second triangle
                            voxel::right_bottom_front,
                            voxel::left_top_front    ,
                            voxel::left_bottom_front ,
            
            // Right face:  First triangle
            voxel::right,   voxel::left_bottom_back  ,
                            voxel::left_top_back     ,
                            voxel::right_top_back    ,
            
            //              Second triangle
                            voxel::left_bottom_back  ,
                            voxel::right_top_back    ,
                            voxel::right_bottom_back ,
            
            // Bottom face: First triangle
            voxel::bottom,  voxel::right_bottom_back ,
                            voxel::right_bottom_front,
                            voxel::left_bottom_front ,
            
            //              Second triangle
                            voxel::right_bottom_back ,
                            voxel::left_bottom_front ,
                            voxel::left_bottom_back  ,
            
            // Top face:    First triangle
            voxel::top,     voxel::right_top_front   ,
                            voxel::right_top_back    ,
                            voxel::left_top_back     ,

            //              Second triangle
                            voxel::right_top_front   ,
                            voxel::left_top_back     ,
                            voxel::left_top_front    ,
            
            // Back face:   First triangle
            voxel::back,    voxel::left_bottom_front ,
                            voxel::left_top_front    ,
                            voxel::left_top_back     ,
            
            //              Second triangle
                            voxel::left_bottom_front ,
                            voxel::left_top_back     ,
                            voxel::left_bottom_back  ,
            
            // Front face:  First triangle
            voxel::front,   voxel::right_bottom_back ,
                            voxel::right_top_back    ,
                            voxel::right_top_front   ,
            
            //              Second triangle
                            voxel::right_bottom_back ,
                            voxel::right_top_front   ,
                            voxel::right_bottom_front
        };
    }
    
    
    constexpr auto normals = make_normals();
    constexpr auto faces   = make_faces();
    
    class obj
    {
    public:
        void cube(voxel v) {
            auto c = v.corners<glm::vec3>();
            
            _indexes.push_back(_vertices.size());
            
            _vertices.insert(_vertices.end(), begin(c), end(c));
        }
        
        void write(std::ostream &os) const {
            for(auto v : _vertices) {
                os << "v " << v.x << " " << v.y << " " << v.z << "\n";
            }
            
            os << "\n";
            
            for(auto n : normals) {
                os << "vn " << n[0] << " " << n[1] << " " << n[2] << "\n";
            }
            
            for (size_t i : _indexes) {
                for(face f : faces) {
                    for(size_t v = 0; v < f.vertices.size(); ++v) {
                        if(v % 3 == 0)
                            os << "\nf ";
                        os << (f.vertices[v] + i + 1) << "//"
                           << (f.normal + 1) << " ";
                    }
                }
            }
        }
        
    private:
        std::vector<glm::vec3> _vertices;
        std::vector<size_t> _indexes;
    };
    
    void obj_mesh(octree const&oc, std::ostream &out)
    {
        obj o;
        for(voxel v : oc) {
            assert(v.material() != voxel::unknown_material);
            if(v.material() != voxel::void_material)
                o.cube(v);
        }
        
        o.write(out);
    }
    
    void octree::mesh(mesh_t mesh_type, std::ostream &out) const {
        switch (mesh_type) {
            case obj:
                obj_mesh(*this, out);
                return;
        }
        assert(!"Unimplemented mesh type");
    }
    
} // namespace details
} // namespace ocmesh
