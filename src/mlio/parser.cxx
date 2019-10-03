/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
inline namespace v1 {
namespace detail {
namespace {

template<bool B>
using return_if = std::enable_if_t<B, parser>;

template<data_type dt>
decltype(auto)
at(device_array_span arr, std::size_t index) noexcept
{
    return arr.as<data_type_t<dt>>()[index];
}

template<data_type dt>
return_if<dt == data_type::size>
make_parser_core(parser_params const &)
{
    return [](stdx::string_view s, device_array_span arr, std::size_t index)
    {
        return try_parse_size_t(s, at<dt>(arr, index));
    };
}

template<data_type dt>
return_if<dt == data_type::float16>
make_parser_core(parser_params const &)
{
    return [](stdx::string_view, device_array_span, std::size_t)
    {
        return parse_result::failed;
    };
}

template<data_type dt>
return_if<dt == data_type::float32 || dt == data_type::float64>
make_parser_core(parser_params const &prm)
{
    return [&prm](stdx::string_view s, device_array_span arr, std::size_t index)
    {
        return try_parse_float({s, &prm.nan_values}, at<dt>(arr, index));
    };
}

template<data_type dt>
return_if<dt == data_type::sint8  ||
          dt == data_type::sint16 ||
          dt == data_type::sint32 ||
          dt == data_type::sint64 ||
          dt == data_type::uint8  ||
          dt == data_type::uint16 ||
          dt == data_type::uint32 ||
          dt == data_type::uint64>
make_parser_core(parser_params const &prm)
{
    return [&prm](stdx::string_view s, device_array_span arr, std::size_t index)
    {
        return try_parse_int({s, prm.base}, at<dt>(arr, index));
    };
}

template<data_type dt>
return_if<dt == data_type::string>
make_parser_core(parser_params const &)
{
    return [](stdx::string_view s, device_array_span arr, std::size_t index)
    {
        at<dt>(arr, index) = static_cast<std::string>(s);

        return parse_result::ok;
    };
}

template<data_type dt>
struct make_parser_op {
    parser
    operator()(parser_params const &prm)
    {
        return make_parser_core<dt>(prm);
    }
};

}  // namespace
}  // namespace detail

parser
make_parser(data_type dt, parser_params const &prm)
{
    return dispatch<detail::make_parser_op>(dt, prm);
}

}  // namespace v1
}  // namespace mlio
