/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <paludis/paludis.hh>
#include <paludis/util/visitor-impl.hh>
#include <sstream>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for PortageDepParser.
 *
 */

namespace test_cases
{
    /**
     * \test Test PortageDepParser with an empty input.
     *
     */
    struct PortageDepParserEmptyTest : TestCase
    {
        PortageDepParserEmptyTest() : TestCase("empty") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            PortageDepParser::parse_depend("",
                    EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "");
        }
    } test_dep_spec_parser_empty;

    /**
     * \test Test PortageDepParser with a blank input.
     *
     */
    struct PortageDepParserBlankTest : TestCase
    {
        PortageDepParserBlankTest() : TestCase("blank") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            PortageDepParser::parse_depend("   \n   \t",
                    EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "");
        }
    } test_dep_spec_parser_blank;

    /**
     * \test Test PortageDepParser with a package.
     *
     */
    struct PortageDepParserPackageTest : TestCase
    {
        PortageDepParserPackageTest() : TestCase("package") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            PortageDepParser::parse_depend("app-editors/vim",
                    EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "app-editors/vim");
        }
    } test_dep_spec_parser_package;

    /**
     * \test Test PortageDepParser with a decorated package.
     *
     */
    struct PortageDepParserDecoratedPackageTest : TestCase
    {
        PortageDepParserDecoratedPackageTest() : TestCase("decorated package") { }

        void run()
        {
            DepSpecPrettyPrinter d1(0, false);
            PortageDepParser::parse_depend(">=app-editors/vim-6.4_alpha",
                    EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d1);
            TEST_CHECK_EQUAL(stringify(d1), ">=app-editors/vim-6.4_alpha");

            DepSpecPrettyPrinter d2(0, false);
            PortageDepParser::parse_depend("=app-editors/vim-6.4_alpha-r1",
                    EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d2);
            TEST_CHECK_EQUAL(stringify(d2), "=app-editors/vim-6.4_alpha-r1");

            DepSpecPrettyPrinter d3(0, false);
            PortageDepParser::parse_depend(">=app-editors/vim-6.4_alpha:one",
                    EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d3);
            TEST_CHECK_EQUAL(stringify(d3), ">=app-editors/vim-6.4_alpha:one");
        }
    } test_dep_spec_parser_decorated_package;

    /**
     * \test Test PortageDepParser with a sequence of packages.
     *
     */
    struct PortageDepParserPackagesTest : TestCase
    {
        PortageDepParserPackagesTest() : TestCase("packages") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            PortageDepParser::parse_depend("app-editors/vim app-misc/hilite   \nsys-apps/findutils",
                    EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "app-editors/vim app-misc/hilite sys-apps/findutils");
        }
    } test_dep_spec_parser_packages;

    struct PortageDepParserAnyTest : TestCase
    {
        PortageDepParserAnyTest() : TestCase("any") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            PortageDepParser::parse_depend("|| ( one/one two/two )",
                    EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "|| ( one/one two/two )");
        }
    } test_dep_spec_parser_any;

    struct PortageDepParserAnyUseTest : TestCase
    {
        PortageDepParserAnyUseTest() : TestCase("any use") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            PortageDepParser::parse_depend("|| ( one/one foo? ( two/two ) )",
                    EAPIData::get_instance()->eapi_from_string("0"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "|| ( one/one foo? ( two/two ) )");

            TEST_CHECK_THROWS(PortageDepParser::parse_depend("|| ( one/one foo? ( two/two ) )",
                    EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);

            DepSpecPrettyPrinter e(0, false);
            PortageDepParser::parse_depend("|| ( one/one ( foo? ( two/two ) ) )",
                    EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(e);
            TEST_CHECK_EQUAL(stringify(e), "|| ( one/one ( foo? ( two/two ) ) )");
        }
    } test_dep_spec_parser_any_use;

    /**
     * \test Test PortageDepParser with an all group.
     *
     */
    struct PortageDepParserAllTest : TestCase
    {
        PortageDepParserAllTest() : TestCase("all") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            PortageDepParser::parse_depend(" ( one/one two/two )    ",
                    EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "one/one two/two");
        }
    } test_dep_spec_parser_all;

    /**
     * \test Test PortageDepParser with a use group.
     *
     */
    struct PortageDepParserUseTest : TestCase
    {
        PortageDepParserUseTest() : TestCase("use") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            PortageDepParser::parse_depend("foo? ( one/one )", EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "foo? ( one/one )");
        }
    } test_dep_spec_parser_use;

    /**
     * \test Test PortageDepParser with an inverse use group.
     *
     */
    struct PortageDepParserInvUseTest : TestCase
    {
        PortageDepParserInvUseTest() : TestCase("!use") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            PortageDepParser::parse_depend("!foo? ( one/one )", EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d);
            TEST_CHECK_EQUAL(stringify(d), "!foo? ( one/one )");
        }
    } test_dep_spec_parser_inv_use;

    /**
     * \test Test PortageDepParser nesting errors.
     *
     */
    struct PortageDepParserBadNestingTest : TestCase
    {
        PortageDepParserBadNestingTest() : TestCase("bad nesting") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            TEST_CHECK_THROWS(PortageDepParser::parse_depend("!foo? ( one/one",
                        EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(PortageDepParser::parse_depend("!foo? ( one/one ) )",
                        EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(PortageDepParser::parse_depend("( ( ( ) )",
                        EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(PortageDepParser::parse_depend("( ( ( ) ) ) )",
                        EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(PortageDepParser::parse_depend(")",
                        EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
        }
    } test_dep_spec_parser_bad_nesting;

    /**
     * \test Test PortageDepParser weird errors.
     *
     */
    struct PortageDepParserBadValuesTest : TestCase
    {
        PortageDepParserBadValuesTest() : TestCase("bad values") { }

        void run()
        {
            DepSpecPrettyPrinter d(0, false);
            TEST_CHECK_THROWS(PortageDepParser::parse_depend("!foo? ||",
                        EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(PortageDepParser::parse_depend("(((",
                        EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(PortageDepParser::parse_depend(")",
                        EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
            TEST_CHECK_THROWS(PortageDepParser::parse_depend("(foo/bar)",
                        EAPIData::get_instance()->eapi_from_string("paludis-1"))->accept(d), DepStringError);
        }
    } test_dep_spec_parser_bad_values;
}

