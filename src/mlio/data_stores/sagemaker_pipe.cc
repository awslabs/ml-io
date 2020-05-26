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

#include "mlio/data_stores/sagemaker_pipe.h"

#include <utility>

#include <fmt/format.h>

#include "mlio/detail/path.h"
#include "mlio/logger.h"
#include "mlio/not_supported_error.h"
#include "mlio/streams/input_stream.h"
#include "mlio/streams/sagemaker_pipe_input_stream.h"

namespace mlio {
inline namespace abi_v1 {

Sagemaker_pipe::Sagemaker_pipe(std::string path,
                               std::chrono::seconds timeout,
                               std::optional<std::size_t> fifo_id,  // NOLINT
                               Compression compression)
    : path_{std::move(path)}, timeout_{timeout}, fifo_id_{fifo_id}, compression_{compression}
{
    detail::validate_file_path(path_);

    if (compression_ == Compression::infer) {
        throw Not_supported_error{
            "The SageMaker pipe channel does not support inferring compression."};
    }
}

Intrusive_ptr<Input_stream> Sagemaker_pipe::open_read() const
{
    logger::info("The SageMaker pipe '{0}' is being opened.", path_);

    auto stream =
        make_sagemaker_pipe_input_stream(path_, timeout_, std::exchange(fifo_id_, std::nullopt));

    if (compression_ == Compression::none) {
        return std::move(stream);
    }
    return make_inflate_stream(std::move(stream), compression_);
}

std::string Sagemaker_pipe::repr() const
{
    return fmt::format("<Sagemaker_pipe path='{0}' compression='{1}'>", path_, compression_);
}

}  // namespace abi_v1
}  // namespace mlio
