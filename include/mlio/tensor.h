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
inline namespace abi_v1 {

/// @addtogroup tensors Tensors
/// @{

/// Represents a vector for dimension data.
using Size_vector = std::vector<std::size_t>;

/// Represents a vector for dimension data that can be negative such as
/// strides.
using Ssize_vector = std::vector<std::ptrdiff_t>;

/// Represents a multi-dimensional array.
///
/// This is an abstract class that only defines the data type and shape
/// of a Tensor. Derived types specify how the Tensor data is laid out
/// in memory.
class MLIO_API Tensor : public Intrusive_ref_counter<Tensor> {
    friend Attribute;

public:
    Tensor(const Tensor &) = delete;

    Tensor &operator=(const Tensor &) = delete;

    Tensor(Tensor &&) = delete;

    Tensor &operator=(Tensor &&) = delete;

    virtual ~Tensor();

    virtual std::string repr() const = 0;

    Data_type data_type() const noexcept
    {
        return data_type_;
    }

    const Size_vector &shape() const noexcept
    {
        return shape_;
    }

    const Ssize_vector &strides() const noexcept
    {
        return strides_;
    }

    /// Invokes the specific visit function for this Tensor type.
    virtual void accept(Tensor_visitor &visitor) = 0;

    virtual void accept(Tensor_visitor &visitor) const = 0;

protected:
    explicit Tensor(Data_type dt, Size_vector &&shape, Ssize_vector &&strides);

private:
    static Ssize_vector default_strides(const Size_vector &shape);

    Data_type data_type_;
    Size_vector shape_;
    Ssize_vector strides_;
};

MLIO_API
inline std::ostream &operator<<(std::ostream &s, const Tensor &tensor)
{
    return s << tensor.repr();
}

/// Represents a Tensor that stores its data in a contiguous memory
/// block.
class MLIO_API Dense_tensor final : public Tensor {
public:
    /// @param data
    ///     A @ref Device_array that holds the data of the Tensor.
    /// @param strides
    ///     An array of the same length as @p shape giving the number
    ///     of elements to skip to get to a new element in each
    ///     dimension.
    explicit Dense_tensor(Size_vector shape,
                          std::unique_ptr<Device_array> &&data,
                          Ssize_vector strides = {});

    std::string repr() const final;

    Device_array_span data() noexcept
    {
        return Device_array_span{*data_};
    }

    Device_array_view data() const noexcept
    {
        return Device_array_view{*data_};
    }

    void accept(Tensor_visitor &visitor) final;

    void accept(Tensor_visitor &visitor) const final;

private:
    void validate_data_size() const;

    std::unique_ptr<Device_array> data_;
};

/// Represents a Tensor that stores its data in coordinate format.
class MLIO_API Coo_tensor final : public Tensor {
public:
    /// @param coordinates
    ///     A vector of indices per dimension indicating where the
    ///     corresponding data element is stored.
    explicit Coo_tensor(Size_vector shape,
                        std::unique_ptr<Device_array> &&data,
                        std::vector<std::unique_ptr<Device_array>> &&coordinates);

    std::string repr() const final;

    Device_array_span data() noexcept
    {
        return Device_array_span{*data_};
    }

    Device_array_view data() const noexcept
    {
        return Device_array_view{*data_};
    }

    Device_array_span indices(std::size_t dim) noexcept
    {
        return *coordinates_.at(dim);
    }

    Device_array_view indices(std::size_t dim) const noexcept
    {
        return *coordinates_.at(dim);
    }

    void accept(Tensor_visitor &visitor) final;

    void accept(Tensor_visitor &visitor) const final;

private:
    std::unique_ptr<Device_array> data_;
    std::vector<std::unique_ptr<Device_array>> coordinates_;
};

/// Represents a Tensor that stores its data as a Compressed Sparse Row
/// matrix.
class MLIO_API Csr_tensor final : public Tensor {
public:
    /// @param data
    ///     A @ref Device_array that holds the data of the Tensor.
    /// @param indices
    ///     The index array of the Tensor.
    /// @param indptr
    ///     The index pointer array of the Tensor.
    explicit Csr_tensor(Size_vector shape,
                        std::unique_ptr<Device_array> &&data,
                        std::unique_ptr<Device_array> &&indices,
                        std::unique_ptr<Device_array> &&indptr);

    std::string repr() const final;

    Device_array_span data() noexcept
    {
        return Device_array_span{*data_};
    }

    Device_array_view data() const noexcept
    {
        return Device_array_view{*data_};
    }

    Device_array_view indices() const noexcept
    {
        return Device_array_view{*indices_};
    }

    Device_array_view indptr() const noexcept
    {
        return Device_array_view{*indptr_};
    }

    void accept(Tensor_visitor &visitor) final;

    void accept(Tensor_visitor &visitor) const final;

private:
    std::unique_ptr<Device_array> data_;
    std::unique_ptr<Device_array> indices_;
    std::unique_ptr<Device_array> indptr_;
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
