import os
import unittest

import mlio
from mlio.integ.numpy import as_numpy
import numpy as np


class TestDataReaderParams(unittest.TestCase):

    def setUp(self):
        self.resources_dir = os.path.join(
            os.path.dirname(os.path.realpath(__file__)), 'resources')

    def test_recordio_protobuf_reader_params(self):
        filename = os.path.join(self.resources_dir, 'test.pbr')
        dataset = [mlio.File(filename)]
        rdr_prm = mlio.DataReaderParams(dataset=dataset,
                                        batch_size=1)
        reader = mlio.RecordIOProtobufReader(rdr_prm)

        example = reader.read_example()
        record = [as_numpy(feature) for feature in example]
        assert record[0].squeeze() == np.array(1)
        assert np.all(record[1].squeeze() == np.array([0,0,0]))

        # Parameters should be reusable
        reader2 = mlio.RecordIOProtobufReader(rdr_prm)
        assert reader2.peek_example()

    def test_csv_reader_params(self):
        filename = os.path.join(self.resources_dir, 'test.csv')
        dataset = [mlio.File(filename)]
        rdr_prm = mlio.DataReaderParams(dataset=dataset,
                                        batch_size=1)
        csv_prm = mlio.CsvReaderParams(header_row_index=None)
        reader = mlio.CsvReader(rdr_prm, csv_prm)

        example = reader.read_example()
        record = [as_numpy(feature) for feature in example]
        assert np.all(np.array(record).squeeze() == np.array([1,0,0,0]))

        reader2 = mlio.CsvReader(rdr_prm, csv_prm)
        assert reader2.peek_example()
