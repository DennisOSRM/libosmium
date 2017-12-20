#ifndef OSMIUM_IO_DETAIL_OUTPUT_FORMAT_HPP
#define OSMIUM_IO_DETAIL_OUTPUT_FORMAT_HPP

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

#include <osmium/handler.hpp>
#include <osmium/io/detail/queue_util.hpp>
#include <osmium/io/error.hpp>
#include <osmium/io/file.hpp>
#include <osmium/io/file_format.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/thread/pool.hpp>

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace osmium {

    namespace io {
        class Header;
    } // namespace io

    namespace io {

        namespace detail {

            class OutputBlock : public osmium::handler::Handler {

            protected:

                std::shared_ptr<osmium::memory::Buffer> m_input_buffer;

                std::shared_ptr<std::string> m_out;

                explicit OutputBlock(osmium::memory::Buffer&& buffer) :
                    m_input_buffer(std::make_shared<osmium::memory::Buffer>(std::move(buffer))),
                    m_out(std::make_shared<std::string>()) {
                }

                // Simple function to convert integer to string. This is much
                // faster than using sprintf, but could be further optimized.
                // See https://github.com/miloyip/itoa-benchmark .
                void output_int(int64_t value) {
                    if (value < 0) {
                        *m_out += '-';
                        value = -value;
                    }

                    char temp[20];
                    char *t = temp;
                    do {
                        *t++ = char(value % 10) + '0';
                        value /= 10;
                    } while (value > 0);

                    const auto old_size = m_out->size();
                    m_out->resize(old_size + (t - temp));
                    char* data = &(*m_out)[old_size];
                    do {
                        *data++ += *--t;
                    } while (t != temp);
                }

            }; // class OutputBlock;

            /**
             * Virtual base class for all classes writing OSM files in different
             * formats.
             *
             * Do not use this class or derived classes directly. Use the
             * osmium::io::Writer class instead.
             */
            class OutputFormat {

            protected:

                osmium::thread::Pool& m_pool;
                future_string_queue_type& m_output_queue;

                /**
                 * Wrap the string into a future and add it to the output
                 * queue.
                 */
                void send_to_output_queue(std::string&& data) {
                    add_to_queue(m_output_queue, std::move(data));
                }

            public:

                OutputFormat(osmium::thread::Pool& pool, future_string_queue_type& output_queue) noexcept :
                    m_pool(pool),
                    m_output_queue(output_queue) {
                }

                OutputFormat(const OutputFormat&) = delete;
                OutputFormat& operator=(const OutputFormat&) = delete;

                OutputFormat(OutputFormat&&) = delete;
                OutputFormat& operator=(OutputFormat&&) = delete;

                virtual ~OutputFormat() noexcept = default;

                virtual void write_header(const osmium::io::Header& /*header*/) {
                }

                virtual void write_buffer(osmium::memory::Buffer&& /*buffer*/) = 0;

                virtual void write_end() {
                }

            }; // class OutputFormat

            /**
             * This factory class is used to create objects that write OSM data
             * into a specified output format.
             *
             * Do not use this class directly. Instead use the osmium::io::Writer
             * class.
             */
            class OutputFormatFactory {

            public:

                using create_output_type = std::function<osmium::io::detail::OutputFormat*(osmium::thread::Pool&, const osmium::io::File&, future_string_queue_type&)>;

            private:

                std::array<create_output_type, static_cast<std::size_t>(file_format::last) + 1> m_callbacks;

                OutputFormatFactory() noexcept = default;

                create_output_type& callbacks(const osmium::io::file_format format) noexcept {
                    return m_callbacks[static_cast<std::size_t>(format)];
                }

                const create_output_type& callbacks(const osmium::io::file_format format) const noexcept {
                    return m_callbacks[static_cast<std::size_t>(format)];
                }

            public:

                static OutputFormatFactory& instance() noexcept {
                    static OutputFormatFactory factory;
                    return factory;
                }

                bool register_output_format(const osmium::io::file_format format, create_output_type&& create_function) {
                    callbacks(format) = std::forward<create_output_type>(create_function);
                    return true;
                }

                std::unique_ptr<osmium::io::detail::OutputFormat> create_output(osmium::thread::Pool& pool, const osmium::io::File& file, future_string_queue_type& output_queue) const {
                    const auto func = callbacks(file.format());
                    if (func) {
                        return std::unique_ptr<osmium::io::detail::OutputFormat>((func)(pool, file, output_queue));
                    }

                    throw unsupported_file_format_error{
                                std::string{"Can not open file '"} +
                                file.filename() +
                                "' with type '" +
                                as_string(file.format()) +
                                "'. No support for writing this format in this program."};
                }

            }; // class OutputFormatFactory

            class BlackholeOutputFormat : public osmium::io::detail::OutputFormat {

            public:

                BlackholeOutputFormat(osmium::thread::Pool& pool, const osmium::io::File& /*file*/, future_string_queue_type& output_queue) :
                    OutputFormat(pool, output_queue) {
                }

                BlackholeOutputFormat(const BlackholeOutputFormat&) = delete;
                BlackholeOutputFormat& operator=(const BlackholeOutputFormat&) = delete;

                BlackholeOutputFormat(BlackholeOutputFormat&&) = delete;
                BlackholeOutputFormat& operator=(BlackholeOutputFormat&&) = delete;

                ~BlackholeOutputFormat() noexcept final = default;

                void write_buffer(osmium::memory::Buffer&& /*buffer*/) final {
                }

            }; // class BlackholeOutputFormat

            // we want the register_output_format() function to run, setting
            // the variable is only a side-effect, it will never be used
            const bool registered_blackhole_output = osmium::io::detail::OutputFormatFactory::instance().register_output_format(osmium::io::file_format::blackhole,
                [](osmium::thread::Pool& pool, const osmium::io::File& file, future_string_queue_type& output_queue) {
                    return new osmium::io::detail::BlackholeOutputFormat(pool, file, output_queue);
            });

            // dummy function to silence the unused variable warning from above
            inline bool get_registered_blackhole_output() noexcept {
                return registered_blackhole_output;
            }

        } // namespace detail

    } // namespace io

} // namespace osmium

#endif // OSMIUM_IO_DETAIL_OUTPUT_FORMAT_HPP
