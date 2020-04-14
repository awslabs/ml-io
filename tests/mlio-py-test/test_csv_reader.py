from typing import List

import numpy as np
import pytest

import mlio
from mlio.integ.numpy import as_numpy


def _test_dedupe_column_names(
        tmpdir,
        input_column_names: List[str],
        input_data: List[int],
        expected_column_names: List[str],
        expected_data: List[int],
        dedupe_column_names: bool = True,
        **kwargs
        ) -> None:

    header_str = ','.join(input_column_names)
    data_str = ','.join(str(x) for x in input_data)
    csv_file = tmpdir.join("test.csv")
    csv_file.write(header_str + '\n' + data_str)

    dataset = [mlio.File(str(csv_file))]
    reader_params = mlio.DataReaderParams(dataset=dataset, batch_size=1)
    csv_params = mlio.CsvParams(
        dedupe_column_names=dedupe_column_names,
        **kwargs
    )
    reader = mlio.CsvReader(reader_params, csv_params)

    example = reader.read_example()
    names = [desc.name for desc in example.schema.attributes]
    assert names == expected_column_names

    record = [as_numpy(feature) for feature in example]
    assert np.all(np.array(record).squeeze() == np.array(expected_data))


def test_dedupe_column_names_no_duplicates(tmpdir):
    _test_dedupe_column_names(
        tmpdir,
        input_column_names=["foo", "bar", "baz"],
        input_data=[1, 2, 3],
        expected_column_names=["foo", "bar", "baz"],
        expected_data=[1, 2, 3]
    )


def test_dedupe_column_names_duplicates(tmpdir):
    _test_dedupe_column_names(
        tmpdir,
        input_column_names=["foo", "bar", "bar", "bar", "foo"],
        input_data=[1, 2, 3, 4, 5],
        expected_column_names=["foo", "bar", "bar_1", "bar_2", "foo_1"],
        expected_data=[1, 2, 3, 4, 5]
    )


def test_dedupe_column_names_duplicates_recursive(tmpdir):
    _test_dedupe_column_names(
        tmpdir,
        input_column_names=["foo", "foo", "foo_1", "foo_1", "bar"],
        input_data=[1, 2, 3, 4, 5],
        expected_column_names=["foo", "foo_1", "foo_1_1", "foo_1_2", "bar"],
        expected_data=[1, 2, 3, 4, 5]
    )


def test_dedupe_column_names_false(tmpdir):
    with pytest.raises(mlio.SchemaError):
        _test_dedupe_column_names(
            tmpdir,
            input_column_names=["foo", "foo", "bar"],
            input_data=[1, 2, 3],
            expected_column_names=["", "", ""],  # placeholder; error expected
            expected_data=[0, 0, 0],  # placeholder
            dedupe_column_names=False
        )


def test_dedupe_column_names_use_columns(tmpdir):
    _test_dedupe_column_names(
        tmpdir,
        input_column_names=["foo", "foo", "bar", "baz"],
        input_data=[1, 2, 3, 4],
        expected_column_names=["bar", "baz"],
        expected_data=[3, 4],
        use_columns={"bar", "baz"}
    )


def test_dedupe_column_names_duplicates_use_columns(tmpdir):
    _test_dedupe_column_names(
        tmpdir,
        input_column_names=["foo", "foo", "bar", "baz"],
        input_data=[1, 2, 3, 4],
        expected_column_names=["foo", "foo_1", "baz"],
        expected_data=[1, 2, 4],
        use_columns={"foo", "baz"}
    )


def test_dedupe_column_names_use_columns_by_index(tmpdir):
    _test_dedupe_column_names(
        tmpdir,
        input_column_names=["foo", "foo", "bar", "baz"],
        input_data=[1, 2, 3, 4],
        expected_column_names=["bar", "baz"],
        expected_data=[3, 4],
        use_columns_by_index={2, 3}
    )


def test_dedupe_column_names_duplicates_use_columns_by_index(tmpdir):
    _test_dedupe_column_names(
        tmpdir,
        input_column_names=["foo", "foo", "bar", "baz"],
        input_data=[1, 2, 3, 4],
        expected_column_names=["foo", "foo_1", "baz"],
        expected_data=[1, 2, 4],
        use_columns_by_index={0, 1, 3}
    )
