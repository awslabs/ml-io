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
inline namespace v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Specifies how the last batch read from a dataset should to be
/// handled if the dataset size is not evenly divisible by the batch
/// size.
enum class last_batch_handling {
    /// Return an @ref example where the size of the batch dimension is
    /// less than the requested batch size.
    none,
    /// Drop the last @ref example.
    drop,
    /// Drop the last @ref example and warn.
    drop_warn,
    /// Pad the feature tensors with zero so that the size of the batch
    /// dimension equals the requested batch size.
    ///
    /// @remark
    ///     The @ref example::padding field will indicate how much
    ///     padding was applied to the @ref example.
    pad,
    /// Pad the feature tensors with zero so that the size of the batch
    /// dimension equals the requested batch size and warn.
    ///
    /// @remark
    ///     The @ref example::padding field will indicate how much
    ///     padding was applied to the @ref example.
    pad_warn
};

/// Specifies how a batch that contains erroneous data should be
/// handled.
enum class bad_batch_handling {
    /// Throw exception.
    error,
    /// Skip the batch.
    skip,
    /// Skip the batch and warn.
    skip_warn,
    /// Skip bad instances, pad the batch to the batch size.
    pad,
    /// Skip bad instances, pad the batch to the batch size, and warn.
    pad_warn
};

/// Contains the parameters that are common to all @ref data_reader
/// "data readers".
struct MLIO_API data_reader_params {
    /// A list of @ref data_store instances that together form the
    /// dataset to read from.
    std::vector<intrusive_ptr<data_store>> dataset{};
    /// A number indicating how many @ref instance "data instances"
    /// should be packed into a single @ref example.
    std::size_t batch_size{};
    /// The number of batches to prefetch in background to accelerate
    /// reading. If zero, defaults to the number of processor cores.
    std::size_t num_prefetched_batches{};
    /// The number of parallel batch reads. If not specified, it equals
    /// to @ref num_prefetched_batches. In case a large number of
    /// batches should be prefetched, this parameter can be used to
    /// avoid thread oversubscription.
    std::size_t num_parallel_reads{};
    /// See @ref last_batch_handling.
    last_batch_handling last_batch_hnd = last_batch_handling::none;
    /// See @ref bad_batch_handling.
    bad_batch_handling bad_batch_hnd = bad_batch_handling::error;
    /// A boolean value indicating whether a warning will be output for
    /// each bad instance.
    bool warn_bad_instances = true;
    /// The number of @ref instance "data instances" to skip from the
    /// beginning of the dataset.
    std::size_t num_instances_to_skip{};
    /// The number of @ref instance "data instances" to read. The rest
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
    /// A boolean value indicating whether to shuffle the @ref instance
    /// "data instances" while reading from the dataset.
    bool shuffle_instances = false;
    /// The number of @ref instance "data instances" to buffer and
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
    /// reshuffled after every @ref data_reader::reset() call.
    bool reshuffle_each_epoch = true;
};

/// Represents an interface for classes that read @ref example "examples"
/// from a dataset in a particular data format.
class MLIO_API data_reader : public intrusive_ref_counter<data_reader> {
public:
    data_reader() noexcept = default;

    data_reader(data_reader const &) = delete;

    data_reader(data_reader &&) = delete;

    virtual ~data_reader();

public:
    data_reader &
    operator=(data_reader const &) = delete;

    data_reader &
    operator=(data_reader &&) = delete;

public:
    /// Returns the @ref schema of the dataset.
    virtual intrusive_ptr<schema const>
    read_schema() = 0;

    /// Returns the next @ref example read from the dataset.
    ///
    /// @remark
    ///     If the end of the dataset is reached, returns an
    ///     @c std::nullptr.
    virtual intrusive_ptr<example>
    read_example() = 0;

    /// Returns the next @ref example read from the dataset without
    /// consuming it.
    virtual intrusive_ptr<example>
    peek_example() = 0;

    /// Resets the state of the reader. Calling @ref read_example()
    /// the next time will start reading from the beginning of the
    /// dataset.
    virtual void
    reset() noexcept = 0;

public:
    /// Gets the number of bytes read from the dataset.
    ///
    /// @remark
    ///     The returned number won't include the size of the discarded
    ///     parts of the dataset such as comment blocks.
    ///
    /// @remark
    ///     The returned number can be greater than expected as ML-IO
    ///     reads ahead the dataset in background.
    virtual std::size_t
    num_bytes_read() const noexcept = 0;
};

/// @}

}  // namespace v1
}  // namespace mlio
