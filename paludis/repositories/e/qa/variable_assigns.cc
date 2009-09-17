/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#include "variable_assigns.hh"
#include <paludis/qa.hh>
#include <paludis/util/log.hh>
#include <pcrecpp.h>
#include <list>
#include <sstream>
#include <map>

using namespace paludis;

namespace
{
    QAMessage
    with_id(QAMessage m, const std::tr1::shared_ptr<const PackageID> & id)
    {
        return id ? m.with_associated_id(id) : m;
    }
}

bool
paludis::erepository::variable_assigns_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::string & content,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using variable_assigns_check on '" + (id ? stringify(*id) : stringify(entry)) + "':");

    pcrecpp::RE r_comment("^\\s*#");
    pcrecpp::RE r_make_line("^\\s*e?make\\b");
    pcrecpp::RE r_make_continuation_line("\\\\\\s*$");
    pcrecpp::RE r_strip_quotes("(\"(\\\\.|[^\"])+\"|'(\\\\.|[^'])+')");

    std::map<std::string, pcrecpp::RE> r_vars;
    r_vars.insert(std::make_pair("CFLAGS", pcrecpp::RE("\\bCFLAGS=")));
    r_vars.insert(std::make_pair("CXXFLAGS", pcrecpp::RE("\\bCXXFLAGS=")));
    r_vars.insert(std::make_pair("CPPFLAGS", pcrecpp::RE("\\bCPPFLAGS=")));
    r_vars.insert(std::make_pair("LDFLAGS", pcrecpp::RE("\\bLDFLAGS=")));
    r_vars.insert(std::make_pair("ASFLAGS", pcrecpp::RE("\\bASFLAGS=")));

    if (id)
        Log::get_instance()->message("e.qa.variable_assigns_check", ll_debug, lc_context) << "variable_assigns '"
            << entry << "', '" << *id << "', '" << name << "'";
    else
        Log::get_instance()->message("e.qa.variable_assigns_check", ll_debug, lc_context) << "variable_assigns '"
            << entry << "', '" << name << "'";

    std::stringstream ff(content);

    std::string s;
    unsigned line_number(0);
    bool in_make_continuation(false);
    while (std::getline(ff, s))
    {
        ++line_number;
        if (s.empty() || r_comment.PartialMatch(s))
            continue;

        r_strip_quotes.GlobalReplace("", &s);

        if (r_make_line.PartialMatch(s))
        {
            if (r_make_continuation_line.PartialMatch(s))
                in_make_continuation = true;
            continue;
        }

        if (in_make_continuation)
        {
            in_make_continuation = r_make_continuation_line.PartialMatch(s);
            continue;
        }

        for (std::map<std::string, pcrecpp::RE>::iterator r(r_vars.begin()),
                 r_end(r_vars.end()) ; r != r_end ; ++r)
            if (r->second.PartialMatch(s))
                reporter.message(with_id(QAMessage(entry, qaml_normal, name, "Attempting to assign to " +
                        r->first + " on line " + stringify(line_number)), id));
    }

    return true;
}
