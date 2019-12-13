# Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"). You
# may not use this file except in compliance with the License. A copy of
# the License is located at
#
#      http://aws.amazon.com/apache2.0/
#
# or in the "license" file accompanying this file. This file is
# distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
# ANY KIND, either express or implied. See the License for the specific
# language governing permissions and limitations under the License.

import atexit
import logging

import mlio._core

from mlio._core import\
    BadBatchHandling,\
    Compression,\
    CooTensor,\
    CorruptFooterError,\
    CorruptHeaderError,\
    CorruptRecordError,\
    CsvReader,\
    CsvReaderParams,\
    DataReader,\
    DataReaderError,\
    DataReaderParams,\
    DataStore,\
    DataType,\
    DenseTensor,\
    Device,\
    DeviceArray,\
    DeviceKind,\
    Example,\
    FeatureDesc,\
    FieldTooLargeError,\
    File,\
    InMemoryStore,\
    InflateError,\
    InputStream,\
    InvalidInstanceError,\
    LastBatchHandling,\
    LogLevel,\
    MaxFieldLengthHandling,\
    MemorySlice,\
    NotSupportedError,\
    ParquetRecordReader,\
    ParserParams,\
    Record,\
    RecordIOProtobufReader,\
    RecordKind,\
    RecordReader,\
    RecordTooLargeError,\
    SageMakerPipe,\
    Schema,\
    SchemaError,\
    StreamError,\
    Tensor,\
    list_files


__all__ = [
    'BadBatchHandling',
    'Compression',
    'CooTensor',
    'CorruptFooterError',
    'CorruptHeaderError',
    'CorruptRecordError',
    'CsvReader',
    'CsvReaderParams',
    'DataReader',
    'DataReaderError',
    'DataReaderParams',
    'DataStore',
    'DataType',
    'DenseTensor',
    'Device',
    'DeviceArray',
    'DeviceKind',
    'Example',
    'FeatureDesc',
    'FieldTooLargeError',
    'File',
    'InMemoryStore',
    'InflateError',
    'InputStream',
    'InvalidInstanceError',
    'LastBatchHandling',
    'LogLevel',
    'MaxFieldLengthHandling',
    'MemorySlice',
    'NotSupportedError',
    'ParquetRecordReader',
    'ParserParams',
    'Record',
    'RecordIOProtobufReader',
    'RecordKind',
    'RecordReader',
    'RecordTooLargeError',
    'SageMakerPipe',
    'Schema',
    'SchemaError',
    'StreamError',
    'Tensor',
    'list_files']


_logger = logging.getLogger("mlio")


def _map_log_level(lvl):
    if lvl == LogLevel.OFF:
        return logging.NOTSET
    if lvl == LogLevel.WARNING:
        return logging.WARNING
    if lvl == LogLevel.INFO:
        return logging.INFO
    if lvl == LogLevel.DEBUG:
        return logging.DEBUG


def _log_message(lvl, msg):
    _logger.log(_map_log_level(lvl), msg)


# We write all log messages coming from the library to the the Python
# 'mlio' logger.
mlio._core.set_log_message_handler(_log_message)

# Make sure that we remove our Python handler once the module gets
# unloaded.
atexit.register(mlio._core.clear_log_message_handler)


def set_log_level(lvl):
    """
    Sets the log level for the ML-IO library.

    Parameters
    ----------
    lvl : LogLevel
        The new log level.
    """
    mlio._core.set_log_level(lvl)

    _logger.setLevel(_map_log_level(lvl))


# We use the default log level used by Python.
set_log_level(LogLevel.WARNING)
