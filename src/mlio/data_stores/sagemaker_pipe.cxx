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

#include "mlio/data_stores/sagemaker_pipe.h"

#include <utility>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "mlio/detail/pathname.h"
#include "mlio/logger.h"
#include "mlio/not_supported_error.h"
#include "mlio/streams/input_stream.h"
#include "mlio/streams/sagemaker_pipe_input_stream.h"

namespace mlio {
inline namespace v1 {

sagemaker_pipe::
sagemaker_pipe(std::string pathname, compression cmp)
    : pathname_{std::move(pathname)}, compression_{cmp}
{
    detail::validate_file_pathname(pathname_);

    if (compression_ == compression::infer) {
        throw not_supported_error{
            "The SageMaker pipe channel does not support inferring "
            "compression."};
    }
}

intrusive_ptr<input_stream>
sagemaker_pipe::
open_read() const
{
    logger::info("The SageMaker pipe '{0}' is being opened.", pathname_);

    auto strm = make_sagemaker_pipe_input_stream(pathname_);

    if (compression_ == compression::none) {
        return std::move(strm);
    }
    return make_inflate_stream(std::move(strm), compression_);
}

std::string
sagemaker_pipe::
repr() const
{
    return fmt::format("<sagemaker_pipe pathname='{0}' compression='{1}'>",
                       pathname_, compression_);
}

}  // namespace v1
}  // namespace mlio
