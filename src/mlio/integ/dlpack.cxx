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

#include "mlio/integ/dlpack.h"

#include <cstdint>
#include <memory>
#include <stdexcept>

#include <dlpack/dlpack.h>

#include "mlio/data_type.h"
#include "mlio/device_array.h"
#include "mlio/device.h"
#include "mlio/not_supported_error.h"
#include "mlio/tensor.h"
#include "mlio/tensor_visitor.h"

namespace mlio {
inline namespace v1 {
namespace detail {
namespace {

inline ::DLDeviceType
as_dl_device(device_kind knd)
{
    if (knd == device_kind::cpu()) {
        return ::kDLCPU;
    }

    throw not_supported_error{"The device kind is not supported by DLPack."};
}

inline ::DLContext
as_dl_context(device dev)
{
    return ::DLContext{as_dl_device(dev.kind()), static_cast<int>(dev.id())};
}

template<data_type dt>
inline ::DLDataType
as_dl_data_type(::DLDataTypeCode code)
{
    auto uint_code = static_cast<std::uint8_t>(code);

    return ::DLDataType{uint_code, 8 * sizeof(data_type_t<dt>), 1};
}

DLDataType
as_dl_data_type(data_type dt)
{
    switch (dt) {
    case data_type::size:
        return as_dl_data_type<data_type::size>   (::kDLUInt);
    case data_type::float16:
        return as_dl_data_type<data_type::float16>(::kDLFloat);
    case data_type::float32:
        return as_dl_data_type<data_type::float32>(::kDLFloat);
    case data_type::float64:
        return as_dl_data_type<data_type::float64>(::kDLFloat);
    case data_type::sint8:
        return as_dl_data_type<data_type::sint8>  (::kDLInt);
    case data_type::sint16:
        return as_dl_data_type<data_type::sint16> (::kDLInt);
    case data_type::sint32:
        return as_dl_data_type<data_type::sint32> (::kDLInt);
    case data_type::sint64:
        return as_dl_data_type<data_type::sint64> (::kDLInt);
    case data_type::uint8:
        return as_dl_data_type<data_type::uint8>  (::kDLUInt);
    case data_type::uint16:
        return as_dl_data_type<data_type::uint16> (::kDLUInt);
    case data_type::uint32:
        return as_dl_data_type<data_type::uint32> (::kDLUInt);
    case data_type::uint64:
        return as_dl_data_type<data_type::uint64> (::kDLUInt);
    case data_type::string:
        throw not_supported_error{
            "The string data type is not supported by DLPack."};
    }

    throw not_supported_error{"The tensor has an unknown data type."};
}

std::int64_t *
cast_shape(size_vector const &shape) noexcept
{
    if (shape.empty()) {
        return nullptr;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<std::int64_t *>(
        reinterpret_cast<std::int64_t const *>(shape.data()));
}

std::int64_t *
cast_strides(ssize_vector const &strides) noexcept
{
    if (strides.empty()) {
        return nullptr;
    }

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<std::int64_t *>(
        reinterpret_cast<std::int64_t const *>(strides.data()));

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
}

// This is the structure that has a cyclic reference to the
// DLManagedTensor it holds. It keeps the associated tensor
// alive until the DLManagedTensor gets deleted.
struct dl_pack_context {
    intrusive_ptr<tensor> tsr{};
    ::DLManagedTensor mt{};
};

struct as_dlpack_op : public tensor_visitor {
    using tensor_visitor::visit;

    void
    visit(tensor &) override
    {
        throw std::invalid_argument{
            "DLPack is only supported for dense tensors."};
    }

    void
    visit(dense_tensor &tsr) override;

    ::DLManagedTensor *mt{};
};

void
as_dlpack_op::
visit(dense_tensor &tsr)
{
    auto ctx = std::make_unique<dl_pack_context>();

    ctx->tsr = wrap_intrusive(&tsr, true);

    ctx->mt.dl_tensor.data = tsr.data().data();
    ctx->mt.dl_tensor.ctx = as_dl_context(tsr.data().get_device());
    ctx->mt.dl_tensor.dtype = as_dl_data_type(tsr.dtype());
    ctx->mt.dl_tensor.ndim = static_cast<int>(tsr.shape().size());
    ctx->mt.dl_tensor.shape = cast_shape(tsr.shape());
    ctx->mt.dl_tensor.strides = cast_strides(tsr.strides());
    ctx->mt.dl_tensor.byte_offset = 0;

    ctx->mt.manager_ctx = ctx.get();

    ctx->mt.deleter = [](::DLManagedTensor *self)
    {
        if (self->manager_ctx != nullptr) {
            delete static_cast<dl_pack_context *>(self->manager_ctx);
        }
    };

    mt = &ctx.release()->mt;
}

}  // namespace
}  // namespace detail

::DLManagedTensor *
as_dlpack(tensor &tsr, std::size_t version)
{
#if SIZE_MAX != UINT64_MAX
    throw not_supported_error{"DLPack is only supported on 64-bit platforms."};
#else
    if (version != DLPACK_VERSION) {
        throw not_supported_error{
            "The requested DLPack version is not supported."};
    }

    detail::as_dlpack_op op{};

    tsr.accept(op);

    return op.mt;
#endif
}

intrusive_ptr<tensor>
as_tensor(::DLManagedTensor *, std::size_t)
{
    throw not_supported_error{"Importing DLPack is not supported yet!"};
}

}  // namespace v1
}  // namespace mlio
