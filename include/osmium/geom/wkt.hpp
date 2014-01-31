#ifndef OSMIUM_GEOM_WKT_HPP
#define OSMIUM_GEOM_WKT_HPP

/*

This file is part of Osmium (http://osmcode.org/osmium).

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

#include <cassert>
#include <iterator>
#include <string>
#include <utility>

#include <osmium/osm/location.hpp>
#include <osmium/geom/factory.hpp>

namespace osmium {

    namespace geom {

        struct wkt_factory_traits {
            typedef std::string point_type;
            typedef std::string linestring_type;
            typedef std::string polygon_type;
            typedef std::string multipolygon_type;
            typedef std::string ring_type;
        };

        class WKTFactory : public GeometryFactory<WKTFactory, wkt_factory_traits> {

            friend class GeometryFactory;

        public:

            WKTFactory() :
                GeometryFactory<WKTFactory, wkt_factory_traits>() {
            }

        private:

            std::string m_str {};
            int m_points {0};

            /* Point */

            point_type make_point(const Location location) {
                std::string str {"POINT("};
                location.as_string(std::back_inserter(str), ' ');
                str += ')';
                return str;
            }

            /* LineString */

            void linestring_start() {
                m_str = "LINESTRING(";
                m_points = 0;
            }

            void linestring_add_location(const Location location) {
                location.as_string(std::back_inserter(m_str), ' ');
                m_str += ',';
                ++m_points;
            }

            linestring_type linestring_finish() {
                if (m_points < 2) {
                    m_str.clear();
                    throw geometry_error("not enough points for linestring");
                } else {
                    assert(!m_str.empty());
                    std::string str;
                    std::swap(str, m_str);
                    str[str.size()-1] = ')';
                    return str;
                }
            }

            /* MultiPolygon */

            bool m_in_polygon = false;
            bool m_first = true;

            void multipolygon_start() {
                m_str = "MULTIPOLYGON(";
                m_in_polygon = false;
            }

            void multipolygon_add_outer_ring() {
                if (m_in_polygon) {
                    m_str += ")),";
                }
                m_str += "((";
                m_in_polygon = true;
                m_first = true;
            }

            void multipolygon_add_inner_ring() {
                m_str += "),(";
                m_first = true;
            }

            void multipolygon_add_location(const osmium::Location location) {
                if (!m_first) {
                    m_str += ",";
                }
                location.as_string(std::back_inserter(m_str), ' ');
                m_first = false;
            }

            multipolygon_type multipolygon_finish() {
                m_in_polygon = false;
                m_first = true;
                m_str += ")))";
                return std::move(m_str);
            }

        }; // class WKTFactory

    } // namespace geom

} // namespace osmium

#endif // OSMIUM_GEOM_WKT_HPP
