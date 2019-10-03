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

#pragma once

#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "mlio/config.h"
#include "mlio/data_type.h"
#include "mlio/device_array.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ref_counter.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup tensors Tensors
/// @{

/// Represents a vector for dimension data.
using size_vector = std::vector<std::size_t>;

/// Represents a vector for dimension data that can be negative such as
/// strides.
using ssize_vector = std::vector<std::ptrdiff_t>;

/// Represents a multi-dimensional array.
///
/// This is an abstract class that only defines the data type and shape
/// of a tensor. Derived types specify how the tensor data is laid out
/// in memory.
class MLIO_API tensor : public intrusive_ref_counter<tensor> {
    friend feature_desc;

protected:
    explicit
    tensor(data_type dt, size_vector &&shape, ssize_vector &&strides);

public:
    tensor(tensor const &) = delete;

    tensor(tensor &&) = delete;

    virtual
   ~tensor();

public:
    tensor &
    operator=(tensor const &) = delete;

    tensor &
    operator=(tensor &&) = delete;

private:
    static ssize_vector
    default_strides(size_vector const &shape);

public:
    virtual std::string
    repr() const = 0;

public:
    data_type
    dtype() const noexcept
    {
        return data_type_;
    }

    size_vector const &
    shape() const noexcept
    {
        return shape_;
    }

    ssize_vector const &
    strides() const noexcept
    {
        return strides_;
    }

public:
    /// Invokes the specific visit function for this tensor type.
    virtual void
    accept(tensor_visitor &vst) = 0;

    virtual void
    accept(tensor_visitor &vst) const = 0;

private:
    data_type data_type_;
    size_vector shape_;
    ssize_vector strides_;
};

MLIO_API
inline std::ostream &
operator<<(std::ostream &strm, tensor const &tsr)
{
    return strm << tsr.repr();
}

/// Represents a tensor that stores its data in a contiguous memory
/// block.
class MLIO_API dense_tensor final : public tensor {
public:
    /// @param data
    ///     A @ref device_array that holds the data of the tensor.
    /// @param strides
    ///     An array of the same length as @p shape giving the number
    ///     of elements to skip to get to a new element in each
    ///     dimension.
    explicit
    dense_tensor(size_vector shape, std::unique_ptr<device_array> &&data,
                 ssize_vector strides = {});

private:
    void
    validate_data_size() const;

public:
    std::string
    repr() const final;

public:
    device_array_span
    data() noexcept
    {
        return device_array_span{*data_};
    }

    device_array_view
    data() const noexcept
    {
        return device_array_view{*data_};
    }

public:
    void
    accept(tensor_visitor &vst) final;

    void
    accept(tensor_visitor &vst) const final;

private:
    std::unique_ptr<device_array> data_;
};

/// Represents a tensor that stores its data in coordinate format.
class MLIO_API coo_tensor final : public tensor {
public:
    /// @param coords
    ///     A vector of indices per dimension indicating where the
    ///     corresponding data element is stored.
    explicit
    coo_tensor(size_vector shape, std::unique_ptr<device_array> &&data,
               std::vector<std::unique_ptr<device_array>> &&coords);

public:
    std::string
    repr() const final;

public:
    device_array_span
    data() noexcept
    {
        return device_array_span{*data_};
    }

    device_array_view
    data() const noexcept
    {
        return device_array_view{*data_};
    }

    device_array_span
    indices(std::size_t dim) noexcept
    {
        return *coordinates_.at(dim);
    }

    device_array_view
    indices(std::size_t dim) const noexcept
    {
        return *coordinates_.at(dim);
    }

public:
    void
    accept(tensor_visitor &vst) final;

    void
    accept(tensor_visitor &vst) const final;

private:
    std::unique_ptr<device_array> data_;
    std::vector<std::unique_ptr<device_array>> coordinates_;
};

/// Represents a tensor that stores its data as a Compressed Sparse Row
/// matrix.
class MLIO_API csr_tensor final : public tensor {
public:
    /// @param data
    ///     A @ref device_array that holds the data of the tensor.
    /// @param indices
    ///     The index array of the tensor.
    /// @param indptr
    ///     The index pointer array of the tensor.
    explicit
    csr_tensor(size_vector shape,
               std::unique_ptr<device_array> &&data,
               std::unique_ptr<device_array> &&indices,
               std::unique_ptr<device_array> &&indptr);

public:
    std::string
    repr() const final;

public:
    device_array_span
    data() noexcept
    {
        return device_array_span{*data_};
    }

    device_array_view
    data() const noexcept
    {
        return device_array_view{*data_};
    }

    device_array_view
    indices() const noexcept
    {
        return device_array_view{*indices_};
    }

    device_array_view
    indptr() const noexcept
    {
        return device_array_view{*indptr_};
    }

public:
    void
    accept(tensor_visitor &vst) final;

    void
    accept(tensor_visitor &vst) const final;

private:
    std::unique_ptr<device_array> data_;
    std::unique_ptr<device_array> indices_;
    std::unique_ptr<device_array> indptr_;
};

/// @}

}  // namespace v1
}  // namespace mlio
