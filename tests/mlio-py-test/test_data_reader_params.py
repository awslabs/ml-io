import os

import numpy as np

import mlio
from mlio.integ.numpy import as_numpy

resources_dir = os.path.join(
    os.path.dirname(os.path.realpath(__file__)), 'resources')


def test_recordio_protobuf_reader_params():
    filename = os.path.join(resources_dir, 'test.pbr')
    dataset = [mlio.File(filename)]
    rdr_prm = mlio.DataReaderParams(dataset=dataset,
                                    batch_size=1)
    reader = mlio.RecordIOProtobufReader(rdr_prm)

    example = reader.read_example()
    record = [as_numpy(feature) for feature in example]
    assert record[0].squeeze() == np.array(1)
    assert np.all(record[1].squeeze() == np.array([0, 0, 0]))

    # Parameters should be reusable
    reader2 = mlio.RecordIOProtobufReader(rdr_prm)
    assert reader2.peek_example()


def test_csv_params():
    filename = os.path.join(resources_dir, 'test.csv')
    dataset = [mlio.File(filename)]
    rdr_prm = mlio.DataReaderParams(dataset=dataset,
                                    batch_size=1)
    csv_prm = mlio.CsvParams(header_row_index=None)
    reader = mlio.CsvReader(rdr_prm, csv_prm)

    example = reader.read_example()
    record = [as_numpy(feature) for feature in example]
    assert np.all(np.array(record).squeeze() == np.array([1, 0, 0, 0]))

    reader2 = mlio.CsvReader(rdr_prm, csv_prm)
    assert reader2.peek_example()


def test_data_reader_params_members():
    filename = os.path.join(resources_dir, 'test.pbr')
    dataset = [mlio.File(filename)]
    rdr_prm = mlio.DataReaderParams(dataset=dataset,
                                    batch_size=1)

    assert rdr_prm.dataset == dataset
    assert rdr_prm.batch_size == 1
    assert rdr_prm.num_prefetched_batches == 0
    assert rdr_prm.num_parallel_reads == 0
    assert rdr_prm.last_batch_handling == mlio.LastBatchHandling.NONE
    assert rdr_prm.bad_batch_handling == mlio.BadBatchHandling.ERROR
    assert rdr_prm.num_instances_to_skip == 0
    assert rdr_prm.num_instances_to_read is None
    assert rdr_prm.shard_index == 0
    assert rdr_prm.num_shards == 0
    assert rdr_prm.shuffle_instances is False
    assert rdr_prm.shuffle_window == 0
    assert rdr_prm.shuffle_seed is None
    assert rdr_prm.reshuffle_each_epoch is True
    assert rdr_prm.subsample_ratio is None

    rdr_prm.batch_size = 2
    assert rdr_prm.batch_size == 2


def test_csv_params_members():
    csv_prm = mlio.CsvParams()

    assert csv_prm.column_names == []
    assert csv_prm.name_prefix == ''
    assert csv_prm.use_columns == set()
    assert csv_prm.use_columns_by_index == set()
    assert csv_prm.default_data_type is None
    assert csv_prm.column_types == {}
    assert csv_prm.column_types_by_index == {}
    assert csv_prm.header_row_index == 0
    assert csv_prm.has_single_header is False
    assert csv_prm.delimiter == ','
    assert csv_prm.quote_char == '"'
    assert csv_prm.comment_char is None
    assert csv_prm.allow_quoted_new_lines is False
    assert csv_prm.skip_blank_lines is True
    assert csv_prm.encoding is None
    assert csv_prm.max_field_length is None
    assert csv_prm.max_field_length_handling == \
        mlio.MaxFieldLengthHandling.ERROR
    assert csv_prm.max_line_length is None
    assert csv_prm.parser_params.nan_values == set()
    assert csv_prm.parser_params.number_base == 10

    csv_prm.header_row_index = None
    assert csv_prm.header_row_index is None

    csv_prm.parser_params.number_base = 2
    assert csv_prm.parser_params.number_base == 2

    '''Due to a shortcoming in pybind11, values cannot be added to container
    types, and updates must instead be made via assignment.'''
    csv_prm.column_types['foo'] = mlio.DataType.STRING  # Doesn't work
    assert csv_prm.column_types == {}

    csv_prm.column_types = {'foo': mlio.DataType.STRING}  # OK
    assert csv_prm.column_types == {'foo': mlio.DataType.STRING}
