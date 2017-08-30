#ifndef OSMIUM_OSM_WAY_HPP
#define OSMIUM_OSM_WAY_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013-2017 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <osmium/memory/collection.hpp>
#include <osmium/memory/item.hpp>
#include <osmium/osm/box.hpp>
#include <osmium/osm/entity.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/osm/node_ref.hpp>
#include <osmium/osm/node_ref_list.hpp>
#include <osmium/osm/object.hpp>

namespace osmium {

    namespace builder {
        template <typename TDerived, typename T>
        class OSMObjectBuilder;
    } // namespace builder

    /**
     * List of node references (id and location) in a way.
     */
    class WayNodeList : public NodeRefList {

    public:

        static constexpr osmium::item_type itemtype = osmium::item_type::way_node_list;

        constexpr static bool is_compatible_to(osmium::item_type t) noexcept {
            return t == itemtype;
        }

        WayNodeList() noexcept :
            NodeRefList(itemtype) {
        }

    }; // class WayNodeList

    static_assert(sizeof(WayNodeList) % osmium::memory::align_bytes == 0, "Class osmium::WayNodeList has wrong size to be aligned properly!");

    class Way : public OSMObject {

        template <typename TDerived, typename T>
        friend class osmium::builder::OSMObjectBuilder;

        Way() noexcept :
            OSMObject(sizeof(Way), osmium::item_type::way) {
        }

    public:

        static constexpr osmium::item_type itemtype = osmium::item_type::way;

        constexpr static bool is_compatible_to(osmium::item_type t) noexcept {
            return t == itemtype;
        }

        WayNodeList& nodes() {
            return osmium::detail::subitem_of_type<WayNodeList>(begin(), end());
        }

        const WayNodeList& nodes() const {
            return osmium::detail::subitem_of_type<const WayNodeList>(cbegin(), cend());
        }

        /**
         * Update all nodes in a way with the ID of the given NodeRef with the
         * location of the given NodeRef.
         */
        void update_node_location(const NodeRef& new_node_ref) {
            for (auto& node_ref : nodes()) {
                if (node_ref.ref() == new_node_ref.ref()) {
                    node_ref.set_location(new_node_ref.location());
                }
            }
        }

        /**
         * Do the nodes in this way form a closed ring?
         */
        bool is_closed() const {
            return nodes().is_closed();
        }

        bool ends_have_same_id() const {
            return nodes().ends_have_same_id();
        }

        bool ends_have_same_location() const {
            return nodes().ends_have_same_location();
        }

        /**
         * Calculate the envelope of this way. If the locations of the nodes
         * are not set, the resulting box will be invalid.
         *
         * Complexity: Linear in the number of nodes.
         */
        osmium::Box envelope() const noexcept {
            return nodes().envelope();
        }

    }; // class Way

    static_assert(sizeof(Way) % osmium::memory::align_bytes == 0, "Class osmium::Way has wrong size to be aligned properly!");

} // namespace osmium

#endif // OSMIUM_OSM_WAY_HPP
