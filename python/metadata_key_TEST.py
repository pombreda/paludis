#!/usr/bin/env python
# vim: set fileencoding=utf-8 sw=4 sts=4 et :

#
# Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License version 2, as published by the Free Software Foundation.
#
# Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA
#

import os

repo_path = os.path.join(os.getcwd(), "metadata_key_TEST_dir/testrepo")
irepo_path = os.path.join(os.getcwd(), "metadata_key_TEST_dir/installed")

from paludis import *
from additional_tests import *

import unittest

Log.instance.log_level = LogLevel.WARNING

class TestCase_01_MetadataKeys(unittest.TestCase):
    def setUp(self):
        self.e = NoConfigEnvironment(repo_path, "/var/empty")
        self.ie = NoConfigEnvironment(irepo_path)
        self.pid = iter(self.e.package_database.fetch_repository("testrepo").package_ids("foo/bar")).next()
        self.ipid = iter(self.ie.package_database.fetch_repository("installed").package_ids("cat-one/pkg-one")).next()

    def test_01_contents(self):
        self.assertEquals(self.pid.find_metadata("CONTENTS"), None)
        self.assert_(isinstance(self.ipid.find_metadata("CONTENTS"), MetadataContentsKey))

    def test_02_installed_time(self):
        self.assertEquals(self.pid.find_metadata("INSTALLED_TIME"), None)
        self.assert_(isinstance(self.ipid.find_metadata("INSTALLED_TIME"), MetadataTimeKey))

    def test_03_repository(self):
        self.assertEquals(self.pid.find_metadata("REPOSITORY"), None)
        self.assert_(isinstance(self.ipid.find_metadata("REPOSITORY"), MetadataStringKey))

    def test_04_keywords(self):
        self.assert_(isinstance(self.pid.find_metadata("KEYWORDS"), MetadataKeywordNameIterableKey))
        self.assertEquals(self.ipid.find_metadata("KEYWORDS"), None)

    def test_05_use(self):
        self.assertEquals(self.pid.find_metadata("USE"), None)
        self.assert_(isinstance(self.ipid.find_metadata("USE"), MetadataUseFlagNameIterableKey))

    def test_06_iuse(self):
        self.assert_(isinstance(self.pid.find_metadata("IUSE"), MetadataIUseFlagIterableKey))
        self.assert_(isinstance(self.ipid.find_metadata("IUSE"), MetadataIUseFlagIterableKey))

    def test_07_inherited(self):
        self.assert_(isinstance(self.pid.find_metadata("INHERITED"), MetadataStringIterableKey))
        self.assert_(isinstance(self.ipid.find_metadata("INHERITED"), MetadataStringIterableKey))

    def test_08_depend(self):
        self.assert_(isinstance(self.pid.find_metadata("DEPEND"), MetadataDependencySpecTreeKey))
        self.assertEquals(self.ipid.find_metadata("DEPEND"), None)

class TestCase_02_MetadataKeys_suclassing(unittest.TestCase):
    def test_01_package_id(self):
        class TestKey(MetadataPackageIDKey):
            def __init__(self):
                MetadataPackageIDKey.__init__(self, "raw", "human", MetadataKeyType.NORMAL)

            def value(self):
                e = NoConfigEnvironment(repo_path, "/var/empty")
                pid = iter(e.package_database.fetch_repository("testrepo").package_ids("foo/bar")).next()
                return pid

        test_metadata_package_id_key(TestKey())

    def test_02_string(self):
        class TestKey(MetadataStringKey):
            def __init__(self):
                MetadataStringKey.__init__(self, "raw", "human", MetadataKeyType.NORMAL)

            def value(self):
                return "str"

        test_metadata_string_key(TestKey())

    def test_03_time(self):
        class TestKey(MetadataTimeKey):
            def __init__(self):
                MetadataTimeKey.__init__(self, "raw", "human", MetadataKeyType.NORMAL)

            def value(self):
                return 123

        test_metadata_time_key(TestKey())


    def test_04_contents(self):
        class TestKey(MetadataContentsKey):
            def __init__(self):
                MetadataContentsKey.__init__(self, "raw", "human", MetadataKeyType.NORMAL)

            def value(self):
                return Contents()

        test_metadata_contents_key(TestKey())

    def test_05_repository_mask_info(self):
        class TestKey(MetadataRepositoryMaskInfoKey):
            def __init__(self):
                MetadataRepositoryMaskInfoKey.__init__(self, "raw", "human", MetadataKeyType.NORMAL)

            def value(self):
                return RepositoryMaskInfo("/foo", ["comment"])

        test_metadata_repository_mask_info_key(TestKey())

    def test_06_keyword_name_iterable(self):
        class TestKey(MetadataKeywordNameIterableKey):
            def __init__(self):
                MetadataKeywordNameIterableKey.__init__(self, "raw", "human", MetadataKeyType.NORMAL)

            def value(self):
                return ["keyword"]

            def pretty_print_flat(self, f):
                return "foo"

        test_metadata_keyword_name_set_key(TestKey())

    def test_07_use_flag_name_iterable(self):
        class TestKey(MetadataUseFlagNameIterableKey):
            def __init__(self):
                MetadataUseFlagNameIterableKey.__init__(self, "raw", "human", MetadataKeyType.NORMAL)

            def value(self):
                return ["use"]

            def pretty_print_flat(self, f):
                return "foo"

        test_metadata_use_flag_name_set_key(TestKey())

    def test_08_iuse_flag_iterable(self):
        class TestKey(MetadataIUseFlagIterableKey):
            def __init__(self):
                MetadataIUseFlagIterableKey.__init__(self, "raw", "human", MetadataKeyType.NORMAL)

            def value(self):
                return [IUseFlag("iuse", IUseFlagParseMode.PERMISSIVE, -1)]

            def pretty_print_flat(self, f):
                return "foo"

            def pretty_print_flat_with_comparison(self, e, pid, f):
                return "foo"

        test_metadata_iuse_flag_set_key(TestKey())

    def test_09_string_iterable(self):
        class TestKey(MetadataStringIterableKey):
            def __init__(self):
                MetadataStringIterableKey.__init__(self, "raw", "human", MetadataKeyType.NORMAL)

            def value(self):
                return ["string"]

            def pretty_print_flat(self, f):
                return "foo"

        test_metadata_string_set_key(TestKey())

    def test_10_license_spec_tree(self):
        class TestKey(MetadataLicenseSpecTreeKey):
            def __init__(self):
                MetadataLicenseSpecTreeKey.__init__(self, "raw", "human", MetadataKeyType.NORMAL)

            def value(self):
                return AllDepSpec()

            def pretty_print(self, f):
                return "str"

            def pretty_print_flat(self, f):
                return "str"

        test_metadata_license_spec_tree_key(TestKey())

    def test_11_provide_spec_tree(self):
        class TestKey(MetadataProvideSpecTreeKey):
            def __init__(self):
                MetadataProvideSpecTreeKey.__init__(self, "raw", "human", MetadataKeyType.NORMAL)

            def value(self):
                return AllDepSpec()

            def pretty_print(self, f):
                return "str"

            def pretty_print_flat(self, f):
                return "str"

        test_metadata_provide_spec_tree_key(TestKey())

    def test_12_dependency_spec_tree(self):
        class TestKey(MetadataDependencySpecTreeKey):
            def __init__(self):
                MetadataDependencySpecTreeKey.__init__(self, "raw", "human", MetadataKeyType.NORMAL)

            def value(self):
                return AllDepSpec()

            def pretty_print(self, f):
                return "str"

            def pretty_print_flat(self, f):
                return "str"

        test_metadata_dependency_spec_tree_key(TestKey())


    def test_13_restrict_spec_tree(self):
        class TestKey(MetadataRestrictSpecTreeKey):
            def __init__(self):
                MetadataRestrictSpecTreeKey.__init__(self, "raw", "human", MetadataKeyType.NORMAL)

            def value(self):
                return AllDepSpec()

            def pretty_print(self, f):
                return "str"

            def pretty_print_flat(self, f):
                return "str"

        test_metadata_restrict_spec_tree_key(TestKey())

    def test_14_fetchable_uri_spec_tree(self):
        class TestKey(MetadataFetchableURISpecTreeKey):
            def __init__(self):
                MetadataFetchableURISpecTreeKey.__init__(self, "raw", "human", MetadataKeyType.NORMAL)

            def value(self):
                return AllDepSpec()

            def pretty_print(self, f):
                return "str"

            def pretty_print_flat(self, f):
                return "str"

            def initial_label(self):
                return URIMirrorsOnlyLabel("foo")

        test_metadata_fetchable_uri_spec_tree_key(TestKey())

    def test_15_simple_uri_spec_tree(self):
        class TestKey(MetadataSimpleURISpecTreeKey):
            def __init__(self):
                MetadataSimpleURISpecTreeKey.__init__(self, "raw", "human", MetadataKeyType.NORMAL)

            def value(self):
                return AllDepSpec()

            def pretty_print(self, f):
                return "str"

            def pretty_print_flat(self, f):
                return "str"

        test_metadata_simple_uri_spec_tree_key(TestKey())


if __name__ == "__main__":
    unittest.main()
