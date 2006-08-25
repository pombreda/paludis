/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
 * Copyright (c) 2006 David Morgan <david.morgan@wadham.oxford.ac.uk>
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

#include <paludis/package_database_entry.hh>
#include <paludis/qa/qa_environment.hh>
#include <paludis/util/collection_concrete.hh>
#include <map>

using namespace paludis;
using namespace paludis::qa;

namespace paludis
{
    template<>
    struct Implementation<QAEnvironmentBase> :
        InternalCounted<QAEnvironmentBase>
    {
        std::map<std::string, PackageDatabase::Pointer> package_databases;

        Implementation(const FSEntry &, const Environment * const env)
        {
            package_databases.insert(std::make_pair("base",
                        PackageDatabase::Pointer(new PackageDatabase(env))));
        }
    };
}

QAEnvironmentBase::QAEnvironmentBase(const FSEntry & b, const Environment * const env) :
    PrivateImplementationPattern<QAEnvironmentBase>(new Implementation<QAEnvironmentBase>(b, env))
{
}

QAEnvironmentBase::~QAEnvironmentBase()
{
}

QAEnvironment::QAEnvironment(const FSEntry & base) :
    QAEnvironmentBase(base, this),
    Environment(_imp->package_databases.begin()->second)
{
    AssociativeCollection<std::string, std::string>::Pointer keys(
            new AssociativeCollection<std::string, std::string>::Concrete);

    keys->insert("format", "portage");
    keys->insert("importace", "1");
    keys->insert("location", stringify(base));
    keys->insert("cache", "/var/empty");
    keys->insert("profile", stringify(base / "profiles" / "base"));

    package_database()->add_repository(
            RepositoryMaker::get_instance()->find_maker("portage")(this,
            package_database().raw_pointer(), keys));
}

QAEnvironment::~QAEnvironment()
{
}

std::string
QAEnvironment::paludis_command() const
{
    return "diefunc 'qa_environment.cc' 'QAEnvironment::paludis_command()' "
        "'paludis_command called from within QAEnvironment'";
}

