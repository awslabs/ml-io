import os

import mlio

resources_dir = os.path.join(
    os.path.dirname(os.path.realpath(__file__)), '../resources/images')


def test_image_reader_jpeg():
    filename = os.path.join(resources_dir, 'test_image_0.jpg')
    dataset = [mlio.File(filename)]
    rdr_prm = mlio.DataReaderParams(dataset=dataset,
                                    batch_size=1)
    img_prm = mlio.ImageReaderParams(img_frame=mlio.ImageFrame.NONE, resize=100, image_dimensions=[3,100,100], to_rgb=1)

    reader = mlio.ImageReader(rdr_prm, img_prm)
    example = reader.read_example()
    tensor = example['value']

    assert tensor.shape == (1, 100, 100, 3)
    assert tensor.strides == (30000, 300, 3, 1)

def test_image_reader_jpeg_no_resize():
    filename = os.path.join(resources_dir, 'test_image_0.jpg')
    dataset = [mlio.File(filename)]
    rdr_prm = mlio.DataReaderParams(dataset=dataset,
                                    batch_size=1)
    img_prm = mlio.ImageReaderParams(img_frame=mlio.ImageFrame.NONE, image_dimensions=[3,50,50], to_rgb=1)

    reader = mlio.ImageReader(rdr_prm, img_prm)
    example = reader.read_example()
    tensor = example['value']

    assert tensor.shape == (1, 50, 50, 3)
    assert tensor.strides == (7500, 150, 3, 1)


def test_image_reader_png():
    filename = os.path.join(resources_dir, 'test_image_0.png')
    dataset = [mlio.File(filename)]
    rdr_prm = mlio.DataReaderParams(dataset=dataset,
                                    batch_size=1)
    img_prm = mlio.ImageReaderParams(img_frame=mlio.ImageFrame.NONE, resize=100, image_dimensions=[3,100,100], to_rgb=1)

    reader = mlio.ImageReader(rdr_prm, img_prm)
    example = reader.read_example()
    tensor = example['value']

    assert tensor.shape == (1, 100, 100, 3)
    assert tensor.strides == (30000, 300, 3, 1)


def test_image_reader_recordio():
    filename = os.path.join(resources_dir, 'test_image_0.rec')
    dataset = [mlio.File(filename)]
    rdr_prm = mlio.DataReaderParams(dataset=dataset,
                                    batch_size=1)
    img_prm = mlio.ImageReaderParams(img_frame=mlio.ImageFrame.RECORDIO, resize=100, image_dimensions=[3,100,100],
                                     to_rgb=1)

    reader = mlio.ImageReader(rdr_prm, img_prm)
    example = reader.read_example()
    tensor = example['value']

    assert tensor.shape == (1, 100, 100, 3)
    assert tensor.strides == (30000, 300, 3, 1)
