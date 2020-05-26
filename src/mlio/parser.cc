/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not use this file except in compliance with the License. A copy of
 * the License is located at
 *
 *      http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

#include "mlio/parser.h"

#include <type_traits>

#include "mlio/util/number.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {
namespace {

template<bool cond>
using Return_if = std::enable_if_t<cond, Parser>;

template<Data_type dt>
decltype(auto) at(Device_array_span arr, std::size_t index) noexcept
{
    return arr.as<data_type_t<dt>>()[index];
}

template<Data_type dt>
Return_if<dt == Data_type::size> make_parser_core(const Parser_params &)
{
    return [](std::string_view s, Device_array_span arr, std::size_t index) {
        return try_parse_size_t(s, at<dt>(arr, index));
    };
}

template<Data_type dt>
Return_if<dt == Data_type::float16> make_parser_core(const Parser_params &)
{
    return [](std::string_view, Device_array_span, std::size_t) {
        return Parse_result::failed;
    };
}

template<Data_type dt>
Return_if<dt == Data_type::float32 || dt == Data_type::float64>
make_parser_core(const Parser_params &params)
{
    return [&params](std::string_view s, Device_array_span arr, std::size_t index) {
        return try_parse_float({s, &params.nan_values}, at<dt>(arr, index));
    };
}

template<Data_type dt>
Return_if<dt == Data_type::int8 || dt == Data_type::int16 || dt == Data_type::int32 ||
          dt == Data_type::int64 || dt == Data_type::uint8 || dt == Data_type::uint16 ||
          dt == Data_type::uint32 || dt == Data_type::uint64>
make_parser_core(const Parser_params &params)
{
    return [&params](std::string_view s, Device_array_span arr, std::size_t index) {
        return try_parse_int({s, params.base}, at<dt>(arr, index));
    };
}

template<Data_type dt>
Return_if<dt == Data_type::string> make_parser_core(const Parser_params &)
{
    return [](std::string_view s, Device_array_span arr, std::size_t index) {
        at<dt>(arr, index) = static_cast<std::string>(s);

        return Parse_result::ok;
    };
}

template<Data_type dt>
struct make_parser_op {
    Parser operator()(const Parser_params &params)
    {
        return make_parser_core<dt>(params);
    }
};

}  // namespace
}  // namespace detail

Parser make_parser(Data_type dt, const Parser_params &params)
{
    return dispatch<detail::make_parser_op>(dt, params);
}

}  // namespace abi_v1
}  // namespace mlio
