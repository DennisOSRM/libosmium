#ifndef OSMIUM_OSM_NODE_REF_HPP
#define OSMIUM_OSM_NODE_REF_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013,2014 Jochen Topf <jochen@topf.org> and others (see README).

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

#include <cstdlib>

#include <osmium/memory/item.hpp>
#include <osmium/osm/item_type.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/osm/types.hpp>

namespace osmium {

    class NodeRef : public osmium::memory::detail::ItemHelper {

        osmium::object_id_type m_ref;
        osmium::Location m_location;

        template <class TMember>
        friend class osmium::memory::CollectionIterator;

        unsigned char* next() {
            return data() + sizeof(NodeRef);
        }

        const unsigned char* next() const {
            return data() + sizeof(NodeRef);
        }

    public:

        static constexpr osmium::item_type collection_type = osmium::item_type::way_node_list;

        NodeRef(const osmium::object_id_type ref=0, const osmium::Location& location=Location()) :
            m_ref(ref),
            m_location(location) {
        }

        osmium::object_id_type ref() const {
            return m_ref;
        }

        osmium::unsigned_object_id_type positive_ref() const {
            return std::abs(m_ref);
        }

        osmium::Location location() const {
            return m_location;
        }

        double lon() const {
            return m_location.lon();
        }

        double lat() const {
            return m_location.lat();
        }

        void location(const osmium::Location& location) {
            m_location = location;
        }

    }; // class NodeRef

    inline bool operator<(const NodeRef& lhs, const NodeRef& rhs) {
        return lhs.ref() < rhs.ref();
    }

    inline bool operator>(const NodeRef& lhs, const NodeRef& rhs) {
        return lhs.ref() > rhs.ref();
    }

    inline bool operator<=(const NodeRef& lhs, const NodeRef& rhs) {
        return lhs.ref() <= rhs.ref();
    }

    inline bool operator>=(const NodeRef& lhs, const NodeRef& rhs) {
        return lhs.ref() >= rhs.ref();
    }

    inline bool operator==(const NodeRef& lhs, const NodeRef& rhs) {
        return lhs.ref() == rhs.ref();
    }

    inline bool operator!=(const NodeRef& lhs, const NodeRef& rhs) {
        return !(lhs == rhs);
    }

    /**
     * Output a NodeRef to a stream.
     */
    template <class T>
    inline T& operator<<(T& out, const osmium::NodeRef& nr) {
        out << "<" << nr.ref() << " " << nr.location() << ">";
        return out;
    }

    /**
     * Functor to compare NodeRefs by Location instead of id.
     */
    struct location_equal {

        bool operator()(const NodeRef& lhs, const NodeRef& rhs) const {
            return lhs.location() == rhs.location();
        }

        typedef NodeRef first_argument_type;
        typedef NodeRef second_argument_type;
        typedef bool result_type;

    }; // struct location_equal

    /**
     * Functor to compare NodeRefs by Location instead of id.
     */
    struct location_less {

        bool operator()(const NodeRef& lhs, const NodeRef& rhs) const {
            return lhs.location() < rhs.location();
        }

        typedef NodeRef first_argument_type;
        typedef NodeRef second_argument_type;
        typedef bool result_type;

    }; // struct location_less

} // namespace osmium

#endif // OSMIUM_OSM_NODE_REF_HPP
