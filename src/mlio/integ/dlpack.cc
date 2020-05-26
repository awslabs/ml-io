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

#include "mlio/integ/dlpack.h"

#include <climits>
#include <cstdint>
#include <memory>
#include <stdexcept>

#include <dlpack/dlpack.h>

#include "mlio/data_type.h"
#include "mlio/device.h"
#include "mlio/not_supported_error.h"
#include "mlio/tensor.h"
#include "mlio/tensor_visitor.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {
namespace {

inline ::DLDeviceType as_dl_device(Device_kind knd)
{
    if (knd == Device_kind::cpu()) {
        return ::kDLCPU;
    }

    throw Not_supported_error{"The device kind is not supported by DLPack."};
}

inline ::DLContext as_dl_context(Device dev)
{
    return ::DLContext{as_dl_device(dev.kind()), static_cast<int>(dev.id())};
}

template<Data_type dt>
inline ::DLDataType as_dl_data_type(::DLDataTypeCode code)
{
    auto uint_code = static_cast<std::uint8_t>(code);

    return ::DLDataType{uint_code, CHAR_BIT * sizeof(data_type_t<dt>), 1};
}

// clang-format off

DLDataType
as_dl_data_type(Data_type dt)
{
    switch (dt) {
    case Data_type::size:
        return as_dl_data_type<Data_type::size>   (::kDLUInt);
    case Data_type::float16:
        return as_dl_data_type<Data_type::float16>(::kDLFloat);
    case Data_type::float32:
        return as_dl_data_type<Data_type::float32>(::kDLFloat);
    case Data_type::float64:
        return as_dl_data_type<Data_type::float64>(::kDLFloat);
    case Data_type::int8:
        return as_dl_data_type<Data_type::int8>   (::kDLInt);
    case Data_type::int16:
        return as_dl_data_type<Data_type::int16>  (::kDLInt);
    case Data_type::int32:
        return as_dl_data_type<Data_type::int32>  (::kDLInt);
    case Data_type::int64:
        return as_dl_data_type<Data_type::int64>  (::kDLInt);
    case Data_type::uint8:
        return as_dl_data_type<Data_type::uint8>  (::kDLUInt);
    case Data_type::uint16:
        return as_dl_data_type<Data_type::uint16> (::kDLUInt);
    case Data_type::uint32:
        return as_dl_data_type<Data_type::uint32> (::kDLUInt);
    case Data_type::uint64:
        return as_dl_data_type<Data_type::uint64> (::kDLUInt);
    case Data_type::string:
        throw Not_supported_error{"The string data type is not supported by DLPack."};
    }

    throw Not_supported_error{"The tensor has an unknown data type."};
}

// clang-format on

std::int64_t *cast_shape(const Size_vector &shape) noexcept
{
    if (shape.empty()) {
        return nullptr;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<std::int64_t *>(reinterpret_cast<const std::int64_t *>(shape.data()));
}

std::int64_t *cast_strides(const Ssize_vector &strides) noexcept
{
    if (strides.empty()) {
        return nullptr;
    }

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<std::int64_t *>(reinterpret_cast<const std::int64_t *>(strides.data()));

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
}

// This is a struct that keeps a cyclic reference to its associated
// DLManagedTensor instance and makes sure that its MLIO tensor stays
// alive until the DLManagedTensor gets deleted.
struct dl_pack_context {
    Intrusive_ptr<Tensor> tensor{};
    ::DLManagedTensor mt{};
};

struct as_dlpack_op : public Tensor_visitor {
    using Tensor_visitor::visit;

    void visit(Tensor &) override
    {
        throw std::invalid_argument{"DLPack is only supported for dense tensors."};
    }

    void visit(Dense_tensor &tensor) override;

    ::DLManagedTensor *mt{};
};

void as_dlpack_op::visit(Dense_tensor &tensor)
{
    auto ctx = std::make_unique<dl_pack_context>();

    ctx->tensor = wrap_intrusive(&tensor, true);

    ctx->mt.dl_tensor.data = tensor.data().data();
    ctx->mt.dl_tensor.ctx = as_dl_context(tensor.data().device());
    ctx->mt.dl_tensor.dtype = as_dl_data_type(tensor.data_type());
    ctx->mt.dl_tensor.ndim = static_cast<int>(tensor.shape().size());
    ctx->mt.dl_tensor.shape = cast_shape(tensor.shape());
    ctx->mt.dl_tensor.strides = cast_strides(tensor.strides());
    ctx->mt.dl_tensor.byte_offset = 0;

    ctx->mt.manager_ctx = ctx.get();

    ctx->mt.deleter = [](::DLManagedTensor *self) {
        if (self->manager_ctx != nullptr) {
            delete static_cast<dl_pack_context *>(self->manager_ctx);
        }
    };

    mt = &ctx.release()->mt;
}

}  // namespace
}  // namespace detail

::DLManagedTensor *as_dlpack(Tensor &tensor, std::size_t version)
{
#if SIZE_MAX != UINT64_MAX
    throw Not_supported_error{"DLPack is only supported on 64-bit platforms."};
#else
    if (version != DLPACK_VERSION) {
        throw Not_supported_error{"The requested DLPack version is not supported."};
    }

    detail::as_dlpack_op op{};

    tensor.accept(op);

    return op.mt;
#endif
}

}  // namespace abi_v1
}  // namespace mlio
