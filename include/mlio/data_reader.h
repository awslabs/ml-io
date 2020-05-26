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
#include <cstdint>
#include <optional>
#include <vector>

#include "mlio/config.h"
#include "mlio/data_stores/data_store.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/intrusive_ref_counter.h"
#include "mlio/schema.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Specifies how the last @ref Example read from a dataset should to be
/// handled if the dataset size is not evenly divisible by the batch
/// size.
enum class Last_example_handling {
    /// Return an @ref Example where the size of the batch dimension is
    /// less than the requested batch size.
    none,
    /// Drop the last @ref Example.
    drop,
    /// Drop the last @ref Example and warn.
    drop_warn,
    /// Pad the feature tensors with zero so that the size of the batch
    /// dimension equals the requested batch size.
    ///
    /// @remark
    ///     The @ref Example::padding field will indicate how much
    ///     padding was applied to the @ref Example.
    pad,
    /// Pad the feature tensors with zero so that the size of the batch
    /// dimension equals the requested batch size and warn.
    ///
    /// @remark
    ///     The @ref Example::padding field will indicate how much
    ///     padding was applied to the @ref Example.
    pad_warn
};

/// Specifies how an Example that contains erroneous data should be
/// handled.
enum class Bad_example_handling {
    /// Throw exception.
    error,
    /// Skip the @ref Example.
    skip,
    /// Skip the @ref Example and warn.
    skip_warn,
    /// Skip bad instances, pad the @ref Example to the batch size.
    pad,
    /// Skip bad instances, pad the @ref Example to the batch size, and
    /// warn.
    pad_warn
};

/// Contains the parameters that are common to all @ref Data_reader
/// "data readers".
struct MLIO_API Data_reader_params {
    /// A list of @ref Data_store instances that together form the
    /// dataset to read from.
    std::vector<Intrusive_ptr<Data_store>> dataset{};
    /// A number indicating how many @ref Instance "data instances"
    /// should be packed into a single @ref Example.
    std::size_t batch_size{};
    /// The number of @ref Example "examples" to prefetch in background
    /// to accelerate reading. If zero, defaults to the number of
    /// processor cores.
    std::size_t num_prefetched_examples{};
    /// The number of parallel reads. If not specified, it equals to
    /// @ref num_prefetched_examples. In case a large number of examples
    /// should be prefetched, this parameter can be used to avoid
    /// thread oversubscription.
    std::size_t num_parallel_reads{};
    /// See @ref Last_example_handling.
    Last_example_handling last_example_handling = Last_example_handling::none;
    /// See @ref Bad_example_handling.
    Bad_example_handling bad_example_handling = Bad_example_handling::error;
    /// A boolean value indicating whether a warning will be output for
    /// each bad Instance.
    bool warn_bad_instances = false;
    /// The number of @ref Instance "data instances" to skip from the
    /// beginning of the dataset.
    std::size_t num_instances_to_skip{};
    /// The number of @ref Instance "data instances" to read. The rest
    /// of the dataset will be ignored.
    std::optional<std::size_t> num_instances_to_read{};
    /// The index of the shard to read.
    std::size_t shard_index{};
    /// The number of shards the dataset should be split into. The
    /// reader will only read 1/num_shards of the dataset.
    std::size_t num_shards{};
    /// A ratio between zero and one indicating how much of the dataset
    /// should be read. The dataset will be sampled based on this
    /// number.
    std::optional<float> sample_ratio{};
    /// A boolean value indicating whether to shuffle the @ref Instance
    /// "data instances" while reading from the dataset.
    bool shuffle_instances = false;
    /// The number of @ref Instance "data instances" to buffer and
    /// sample from. The selected data instances will be replaced with
    /// new data instances read from the dataset.
    ///
    /// A value of zero means perfect shuffling and requires loading the
    /// whole dataset into memory first.
    std::size_t shuffle_window{};
    /// The seed that will be used for initializing the sampling
    /// distribution. If not specified, a random seed will be generated
    /// internally
    std::optional<std::uint_fast64_t> shuffle_seed{};
    /// A boolean value indicating whether the dataset should be
    /// reshuffled after every @ref Data_reader::reset() call.
    bool reshuffle_each_epoch = true;
};

/// Represents an interface for classes that read @ref Example "examples"
/// from a dataset in a particular data format.
class MLIO_API Data_reader : public Intrusive_ref_counter<Data_reader> {
public:
    Data_reader() noexcept = default;

    Data_reader(const Data_reader &) = delete;

    Data_reader &operator=(const Data_reader &) = delete;

    Data_reader(Data_reader &&) = delete;

    Data_reader &operator=(Data_reader &&) = delete;

    virtual ~Data_reader();

    /// Returns the @ref Schema of the dataset.
    virtual Intrusive_ptr<const Schema> read_schema() = 0;

    /// Returns the next @ref Example read from the dataset.
    ///
    /// @remark
    ///     If the end of the dataset is reached, returns an
    ///     @c std::nullptr.
    virtual Intrusive_ptr<Example> read_example() = 0;

    /// Returns the next @ref Example read from the dataset without
    /// consuming it.
    virtual Intrusive_ptr<Example> peek_example() = 0;

    /// Resets the state of the reader. Calling @ref read_example()
    /// the next time will start reading from the beginning of the
    /// dataset.
    virtual void reset() noexcept = 0;

    /// Gets the number of bytes read from the dataset.
    ///
    /// @remark
    ///     The returned number won't include the size of the discarded
    ///     parts of the dataset such as Record headers.
    ///
    /// @remark
    ///     The returned number can be greater than expected as MLIO
    ///     can read ahead in background.
    virtual std::size_t num_bytes_read() const noexcept = 0;
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
