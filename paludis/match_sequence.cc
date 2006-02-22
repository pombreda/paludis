/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "match_sequence.hh"

using namespace paludis;

/* gcc 3.4 gets things horribly wrong, and thinks that some functions will
 * never return when in fact they will. this is a nasty workaround. */
#if defined(__GNUC__)
#  if __GNUC__ < 4
#    define WORK_AROUND_BROKEN_COMPILER do { std::string s; \
        if (! s.empty()) return s.size(); } while (false)
#  else
#    define WORK_AROUND_BROKEN_COMPILER
#  endif
#else
#  define WORK_AROUND_BROKEN_COMPILER
#endif

struct MatchRule::Rule :
    InternalCounted<MatchRule::Rule>
{
    virtual std::string::size_type local_match(const std::string &,
            std::string::size_type) const = 0;

    virtual ~Rule()
    {
    }
};

struct MatchRule::StringRule :
    MatchRule::Rule
{
    const std::string s;

    StringRule(const std::string & ss) :
        s(ss)
    {
    }

    std::string::size_type
    local_match(const std::string & t, std::string::size_type p) const
    {
        return (0 == t.compare(p, s.length(), s)) ? s.length() : std::string::npos;
    }
};

struct MatchRule::SequenceRule :
    MatchRule::Rule
{
    const MatchRule r1, r2;

    SequenceRule(const MatchRule & rr1, const MatchRule & rr2) :
        r1(rr1),
        r2(rr2)
    {
    }

    std::string::size_type
    local_match(const std::string & t, std::string::size_type p) const
    {
        WORK_AROUND_BROKEN_COMPILER;

        std::string::size_type l1(r1._rule->local_match(t, p)), l2(0);
        if (std::string::npos == l1)
            return l1;

        l2 = r2._rule->local_match(t, p + l1);
        return (std::string::npos == l2) ? l2 : l1 + l2;
    }
};

struct MatchRule::EitherRule :
    MatchRule::Rule
{
    const MatchRule r1, r2;

    EitherRule(const MatchRule & rr1, const MatchRule & rr2) :
        r1(rr1),
        r2(rr2)
    {
    }

    std::string::size_type
    local_match(const std::string & t, std::string::size_type p) const
    {
        WORK_AROUND_BROKEN_COMPILER;

        std::string::size_type l1(r1._rule->local_match(t, p));
        if (std::string::npos != l1)
            return l1;
        return r2._rule->local_match(t, p);
    }
};

struct MatchRule::ZeroOrMoreRule :
    MatchRule::Rule
{
    const MatchRule r1;

    ZeroOrMoreRule(const MatchRule & rr1) :
        r1(rr1)
    {
    }

    std::string::size_type
    local_match(const std::string & t, std::string::size_type p) const
    {
        WORK_AROUND_BROKEN_COMPILER;

        std::string::size_type result(p), q;
        while (std::string::npos != ((q = r1._rule->local_match(t, result))))
            result += q;

        return result - p;
    }
};

struct MatchRule::EolRule :
    MatchRule::Rule
{
    std::string::size_type
    local_match(const std::string & t, std::string::size_type p) const
    {
        return p >= t.length() ? 0 : std::string::npos;
    }
};

MatchRule::MatchRule(const std::string & s) :
    _rule(CountedPtr<Rule>(new StringRule(s)))
{
}

MatchRule::MatchRule(const MatchRule & other) :
    _rule(other._rule)
{
}

MatchRule::MatchRule(CountedPtr<Rule> r) :
    _rule(r)
{
}

const MatchRule
MatchRule::eol()
{
    return MatchRule(CountedPtr<Rule>(new EolRule));
}

MatchRule::~MatchRule()
{
}

const MatchRule
MatchRule::operator>> (const MatchRule & other) const
{
    return MatchRule(CountedPtr<Rule>(new SequenceRule(*this, other)));
}

const MatchRule
MatchRule::operator|| (const MatchRule & other) const
{
    return MatchRule(CountedPtr<Rule>(new EitherRule(*this, other)));
}

const MatchRule
MatchRule::operator* () const
{
    return MatchRule(CountedPtr<Rule>(new ZeroOrMoreRule(*this)));
}

bool
MatchRule::match(const std::string & s) const
{
    WORK_AROUND_BROKEN_COMPILER;
    return (std::string::npos != _rule->local_match(s, 0));
}

