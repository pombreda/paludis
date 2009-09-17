/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_QA_QA_CONTROLLER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_QA_QA_CONTROLLER_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/qa-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <tr1/functional>
#include <tr1/memory>

namespace paludis
{
    class ERepository;

    namespace erepository
    {
        class PALUDIS_VISIBLE QAController :
            private PrivateImplementationPattern<QAController>
        {
            private:
                void _check_eclasses(const FSEntry &, const std::string &);
                void _check_category(const CategoryNamePart, const std::tr1::shared_ptr<const QualifiedPackageNameSet>);
                void _check_package(const QualifiedPackageName);
                void _check_id(const std::tr1::shared_ptr<const PackageID> &);

                void _worker();
                void _status_worker();

                bool _under_base_dir(const FSEntry &) const;
                bool _above_base_dir(const FSEntry &) const;

            public:
                QAController(
                        const Environment * const,
                        const std::tr1::shared_ptr<const ERepository> &,
                        const QACheckProperties & ignore_if,
                        const QACheckProperties & ignore_unless,
                        const QAMessageLevel minimum_level,
                        QAReporter &,
                        const FSEntry &);

                ~QAController();

                void run();
        };
    }
}

#endif