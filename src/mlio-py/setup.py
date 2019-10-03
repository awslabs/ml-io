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

import os
import platform

from distutils import sysconfig
from setuptools import setup, find_packages
from setuptools.command.install import install as base_install
from setuptools.command.install_lib import install_lib as base_install_lib
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


# We inject our own install_lib command that copies our pre-built
# extension modules into the distribution.
class install_lib(base_install_lib):
    ext_suffix = sysconfig.get_config_var('EXT_SUFFIX')

    def _get_ext_src_path(self, ext_name):
        pathname = ext_name + install_lib.ext_suffix
        if os.path.exists(pathname):
            return pathname

        raise RuntimeError("The extension '%s' is not found." % ext_name)

    def _get_ext_ins_path(self, ext_name):
        return os.path.join(self.install_dir, self._get_ext_src_path(ext_name))

    def _copy_ext_module(self, ext_name):
        src_pathname = self._get_ext_src_path(ext_name)
        ins_pathname = self._get_ext_ins_path(ext_name)

        self.copy_file(src_pathname, ins_pathname)

        # Remove the build tree RPATHs.
        if platform.system() == 'Darwin':
            pass
        else:
            os.system('chrpath --delete ' + ins_pathname)

        return ins_pathname

    def install(self):
        lst = base_install_lib.install(self)

        lst.append(self._copy_ext_module('mlio/core'))
        lst.append(self._copy_ext_module('mlio/integ/arrow'))

        return lst

    def get_outputs(self):
        lst = base_install_lib.get_outputs(self)

        lst.append(self._get_ext_ins_path('mlio/core'))
        lst.append(self._get_ext_ins_path('mlio/integ/arrow'))

        return lst

    def get_inputs(self):
        lst = base_install_lib.get_inputs(self)

        lst.append(self._get_ext_src_path('mlio/core'))
        lst.append(self._get_ext_src_path('mlio/integ/arrow'))

        return lst


# Read the long description.
with open(os.path.join(os.path.dirname(__file__), 'README.md')) as f:
    long_description = f.read()


setup(
    distclass=Distribution,

    # Metadata
    name='mlio',
    version='0.1.1',
    description='A high performance data access library for machine learning tasks',  # noqa: E501
    long_description=long_description,
    long_description_content_type='text/markdown',
    author='Amazon Web Services',
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
        'Topic :: Scientific/Engineering'
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
        'pyarrow': ['pyarrow==0.14.1'],
    },
    cmdclass={
        'install': install,
        'install_lib': install_lib,
    })
