/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_PORTAGE_PORTAGE_REPOSITORY_METADATA_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_PORTAGE_PORTAGE_REPOSITORY_METADATA_HH 1

#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/version_metadata.hh>
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/instantiation_policy.hh>
#include <string>

/** \file
 * Declaration for the PortageRepositoryMetadata class.
 *
 * \ingroup grpportagerepository
 */

namespace paludis
{
    class PortageRepositoryMetadata :
        public InternalCounted<PortageRepositoryMetadata>
    {
        private:
            const std::string & _ext;

        protected:
            PortageRepositoryMetadata(const std::string & ext);

        public:
            virtual ~PortageRepositoryMetadata();

            std::string file_extension() const
            {
                return _ext;
            }

            virtual VersionMetadata::Pointer generate_version_metadata(const QualifiedPackageName &,
                    const VersionSpec &) const = 0;
    };
}

#endif
