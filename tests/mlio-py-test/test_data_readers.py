import os

import mlio
from mlio.integ.numpy import as_numpy

resources_dir = os.path.join(
    os.path.dirname(os.path.realpath(__file__)), '../resources')


def test_text_line_reader_params():
    expected_string = "this is line 1"
    filename = os.path.join(resources_dir, 'test.txt')
    dataset = [mlio.File(filename)]
    rdr_prm = mlio.DataReaderParams(dataset=dataset,
                                    batch_size=1)

    reader = mlio.TextLineReader(rdr_prm)
    example = reader.read_example()
    record = [as_numpy(feature) for feature in example]

    assert record[0] == expected_string
