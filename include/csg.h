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

#ifndef OCMESH_CSG_H__
#define OCMESH_CSG_H__

#include "meta.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <std14/memory>
#include <iterator>
#include <vector>

namespace ocmesh {

    namespace details {
        /*
         * User facing primitives and operations.
         */
        class CSG {
        public:
            virtual ~CSG();
            
            virtual float distance(glm::vec3 const& from) = 0;
        };
        
        using ptr = std::unique_ptr<CSG>;
        
        class Scene {
        public:
            Scene() = default;
            Scene(Scene const&) = default;
            Scene(Scene     &&) = default;
            
            Scene& operator=(Scene const&) = default;
            Scene& operator=(Scene     &&) = default;
            
            
            
        private:
            std::vector<ptr> _objects;
        };
        
        class sphere_t : public CSG
        {
            float _radius;
            
        public:
            sphere_t(float radius) : _radius(radius) { }
            
            float radius() const { return _radius; }
            
            float distance(glm::vec3 const& from) override;
        };
        
        class cube_t : public CSG
        {
            float _side;
            
        public:
            cube_t(float side) : _side(side) { }
            
            float side() const { return _side; }
            
            float distance(glm::vec3 const& from) override;
        };
        
        class binary_operation_t : public CSG {
            ptr _left;
            ptr _right;
            
        public:
            binary_operation_t(ptr left, ptr right)
                : _left(std::move(left)), _right(std::move(right)) { }
            
            CSG *left()  const { return _left.get();  }
            CSG *right() const { return _right.get(); }
        };
        
        class union_t : public binary_operation_t {
        public:
            using binary_operation_t::binary_operation_t;
            
            float distance(glm::vec3 const& from) override;
        };
        
        class intersection_t : public binary_operation_t {
        public:
            using binary_operation_t::binary_operation_t;
            
            float distance(glm::vec3 const& from) override;
        };
        
        class difference_t : public binary_operation_t {
        public:
            using binary_operation_t::binary_operation_t;
            
            float distance(glm::vec3 const& from) override;
        };
        
        class transform_t : public CSG
        {
            ptr       _child;
            glm::mat4 _transform;
            
        public:
            transform_t(ptr child, glm::mat4 const&transform)
                : _child(std::move(child)), _transform(transform) { }
            
            CSG *child() const { return _child.get(); }
            glm::mat4 const&transform() const { return _transform; }
            
            float distance(glm::vec3 const& from) override;
        };
        
        
        // Primitives
        inline ptr sphere(float radius) {
            return std14::make_unique<sphere_t>(radius);
        }
        
        inline ptr cube(float side) {
            return std14::make_unique<cube_t>(side);
        }
        
        // Combining nodes
        inline ptr unite(ptr left, ptr right) {
            return std14::make_unique<union_t>(std::move(left),
                                               std::move(right));
        }
        
        template<typename ...Args>
        inline ptr unite(ptr node, Args&& ...args) {
            return unite(std::move(node), unite(std::forward<Args>(args)...));
        }
        
        inline ptr intersect(ptr left, ptr right) {
            return std14::make_unique<intersection_t>(std::move(left),
                                                      std::move(right));
        }
        
        template<typename ...Args>
        inline ptr intersect(ptr node, Args&& ...args) {
            return intersect(std::move(node),
                             unite(std::forward<Args>(args)...));
        }
        
        inline ptr subtract(ptr left, ptr right) {
            return std14::make_unique<difference_t>(std::move(left),
                                                    std::move(right));
        }
        
        template<typename ...Args>
        inline ptr subtract(ptr node, Args&& ...args) {
            return subtract(std::move(node),
                            unite(std::forward<Args>(args)...));
        }
        
        // Transform nodes
        // Use this function only if you know what you're doing.
        // Use scale/rotate/translate instead.
        // The matrix must be a transform from object to world space
        inline ptr transform(ptr node, glm::mat4 const&matrix) {
            return std14::make_unique<transform_t>(std::move(node), matrix);
        }
        
        inline ptr scale(ptr node, glm::vec3 const&factors) {
            assert(glm::all(glm::notEqual(factors, {})) &&
                   "Scaling factor components must be non-zero");
            return transform(std::move(node),
                             glm::scale(glm::vec3(1, 1, 1) / factors));
        }
        
        inline ptr scale(ptr node, float factor) {
            assert(factor != 0 && "Scaling factor must be non-zero");
            return scale(std::move(node), { factor, factor, factor });
        }
        
        inline ptr xscale(ptr node, float factor) {
            assert(factor != 0 && "Scaling factor must be non-zero");
            return scale(std::move(node), {factor, 1, 1});
        }
        
        inline ptr yscale(ptr node, float factor) {
            assert(factor != 0 && "Scaling factor must be non-zero");
            return scale(std::move(node), {1, factor, 1});
        }
        
        inline ptr zscale(ptr node, float factor) {
            assert(factor != 0 && "Scaling factor must be non-zero");
            return scale(std::move(node), {1, 1, factor});
        }
        
        inline ptr rotate(ptr node, float angle, glm::vec3 const&axis) {
            return transform(std::move(node), glm::rotate(-angle, axis));
        }
        
        inline ptr xrotate(ptr node, float angle) {
            return rotate(std::move(node), angle, {1, 0, 0});
        }
        
        inline ptr yrotate(ptr node, float angle) {
            return rotate(std::move(node), angle, {0, 1, 0});
        }
        
        inline ptr zrotate(ptr node, float angle) {
            return rotate(std::move(node), angle, {0, 0, 1});
        }
        
        inline ptr translate(ptr node, glm::vec3 const&offsets) {
            return transform(std::move(node), glm::translate(-offsets));
        }
        
        inline ptr xtranslate(ptr node, float offset) {
            return translate(std::move(node), {offset, 0, 0});
        }
        
        inline ptr ytranslate(ptr node, float offset) {
            return translate(std::move(node), {0, offset, 0});
        }
        
        inline ptr ztranslate(ptr node, float offset) {
            return translate(std::move(node), {0, 0, offset});
        }
    }
    
    namespace csg {
        using details::CSG;
        using details::ptr;
        using details::sphere;
        using details::cube;
        
        using details::unite;
        using details::intersect;
        using details::subtract;
        using details::transform;
        
        using details::scale;
        using details::xscale;
        using details::yscale;
        using details::zscale;
        
        using details::rotate;
        using details::xrotate;
        using details::yrotate;
        using details::zrotate;
        
        using details::translate;
        using details::xtranslate;
        using details::ytranslate;
        using details::ztranslate;
        
    }
}

#endif
