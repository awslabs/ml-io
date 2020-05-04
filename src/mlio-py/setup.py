# Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

import os
import sysconfig

from setuptools import setup, find_packages
from setuptools.command.install import install as base_install
from setuptools.dist import Distribution as BaseDistribution


class Distribution(BaseDistribution):
    # As we are injecting our extension modules into the package instead
    # of building them we need to mark the distribution as non-pure.
    def has_ext_modules(self):
        return True


class install(base_install):
    def finalize_options(self):
        base_install.finalize_options(self)

        # distutils install command incorrectly checks `ext_modules` to
        # determine whether a distribution is non-pure. We override that
        # behavior here.
        if self.distribution.has_ext_modules():
            self.install_lib = self.install_platlib


def stamp_dev_version(version):
    if version.endswith('.dev'):
        version += os.getenv('MLIO_BUILD_NUMBER', '0+local')
    return version


# Read the long description.
with open(os.path.join(os.path.dirname(__file__), 'README.md')) as f:
    long_description = f.read()


setup(distclass=Distribution,
      cmdclass={
          'install': install,
      },

      # Metadata
      name='mlio',
      version=stamp_dev_version('0.5.2.dev'),
      description='A high performance data access library for machine learning tasks',  # noqa: E501
      long_description=long_description,
      long_description_content_type='text/markdown',
      author='Amazon Web Services',
      maintainer='Can Balioglu',
      maintainer_email='balioglu@amazon.com',
      url='https://github.com/awslabs/ml-io',
      license='Apache License 2.0',
      keywords='ML Amazon AWS AI',
      classifiers=[
          'Development Status :: 4 - Beta',
          'Intended Audience :: Developers',
          'Natural Language :: English',
          'License :: OSI Approved :: Apache Software License',
          'Programming Language :: Python',
          'Programming Language :: Python :: 3',
          'Programming Language :: Python :: 3.6',
          'Programming Language :: Python :: 3.7',
          'Topic :: Scientific/Engineering',
      ],

      # Options
      packages=find_packages(),
      python_requires='>=3.6',
      install_requires=['numpy>=1.8.2'],
      extras_require={
          'pandas': ['pandas>=0.23.0'],
          'scipy': ['scipy>=0.13.3'],
          'tensorflow': ['tensorflow>=1.9.0'],
          'torch': ['torch>=1.0.0'],
          'mxnet': ['mxnet>=1.4.1'],
          'pyarrow': ['pyarrow==0.16.0'],
      },

      # Add our externally-built extension modules to the package.
      package_data={
          '': ['**' + sysconfig.get_config_var('EXT_SUFFIX')]
      })
