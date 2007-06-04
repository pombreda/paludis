/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_INSTALLED_GEMS_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_INSTALLED_GEMS_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/repositories/gems/params-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    /**
     * Repository for installed Gem packages.
     *
     * \ingroup grpgemsrepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE InstalledGemsRepository :
        public Repository,
        public RepositoryDestinationInterface,
        public RepositoryInstalledInterface,
        public RepositoryUninstallableInterface,
        private PrivateImplementationPattern<InstalledGemsRepository>
    {
        private:
            void need_category_names() const;
            void need_entries() const;
            void need_version_metadata(const QualifiedPackageName &, const VersionSpec &) const;

        protected:
            /* core */

            virtual bool do_has_category_named(const CategoryNamePart &) const;

            virtual bool do_has_package_named(const QualifiedPackageName &) const;

            virtual tr1::shared_ptr<const CategoryNamePartCollection> do_category_names() const;

            virtual tr1::shared_ptr<const QualifiedPackageNameCollection> do_package_names(
                    const CategoryNamePart &) const;

            virtual tr1::shared_ptr<const VersionSpecCollection> do_version_specs(
                    const QualifiedPackageName &) const;

            virtual bool do_has_version(const QualifiedPackageName &, const VersionSpec &) const;

            virtual tr1::shared_ptr<const VersionMetadata> do_version_metadata(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            /* uninstallable */

            virtual void do_uninstall(const QualifiedPackageName &, const VersionSpec &,
                    const UninstallOptions &) const;

        public:
            /**
             * Constructor.
             */
            InstalledGemsRepository(const gems::InstalledRepositoryParams &);

            /**
             * Destructor.
             */
            ~InstalledGemsRepository();

            virtual void invalidate();

            /* destination */

            virtual bool is_suitable_destination_for(const PackageDatabaseEntry &) const;
            virtual bool is_default_destination() const;
            virtual bool want_pre_post_phases() const;
            virtual void merge(const MergeOptions &) PALUDIS_ATTRIBUTE((noreturn));

            /* installed */
            virtual FSEntry root() const;

    };
}


#endif
