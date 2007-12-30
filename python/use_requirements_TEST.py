#!/usr/bin/env python
# vim: set fileencoding=utf-8 sw=4 sts=4 et :

#
# Copyright (c) 2007 Piotr Jaroszyński
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

from paludis import *
import unittest

class TestCase_1_UseRequirements(unittest.TestCase):
    def setUp(self):
        self.e = TestEnvironment()
        self.r = FakeRepository(self.e, "fake")
        self.pid = self.r.add_version("cat/pkg", "1")

        self.ur1 = EnabledUseRequirement("foo")
        self.ur2 = DisabledUseRequirement("foo")
        self.ur3 = EqualUseRequirement("foo", self.pid)
        self.ur4 = NotEqualUseRequirement("foo", self.pid)

    def test_01_create(self):
        self.assertRaises(Exception, UseRequirement)

    def test_02_flag(self):
        self.assertEquals(self.ur1.flag(), UseFlagName("foo"))
        self.assertEquals(self.ur2.flag(), UseFlagName("foo"))
        self.assertEquals(self.ur3.flag(), UseFlagName("foo"))
        self.assertEquals(self.ur4.flag(), UseFlagName("foo"))

    def test_03_package_id(self):
        self.assertEquals(self.ur3.package_id(), self.pid)
        self.assertEquals(self.ur4.package_id(), self.pid)


if __name__ == "__main__":
    unittest.main()
