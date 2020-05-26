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

namespace mlio {
inline namespace abi_v1 {
namespace detail {

class Chunk_reader;
class Coo_tensor_builder;
class Iconv_desc;
class Instance_batch_reader;
class Instance_reader;
class Zlib_inflater;

}  // namespace detail

class Attribute;
class Coo_tensor;
class Csr_tensor;
class Data_store;
class Dense_tensor;
class Example;
class Input_stream;
class Instance;
class Instance_batch;
class Memory_slice;
class Mutable_memory_block;
class Record;
class Record_reader;
class Tensor;
class Tensor_visitor;
class Text_encoding;

struct Csv_params;
struct Data_reader_params;

}  // namespace abi_v1
}  // namespace mlio
