/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/repositories/repository/repository_id.hh>
#include <paludis/repositories/repository/repository_repository.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/unchoices_key.hh>
#include <paludis/user_dep_spec.hh>

using namespace paludis;
using namespace paludis::repository_repository;

namespace paludis
{
    template <>
    struct Implementation<RepositoryID>
    {
        const Environment * const env;
        const QualifiedPackageName name;
        const VersionSpec version;
        const RepositoryRepository * const repo;

        const std::tr1::shared_ptr<LiteralMetadataValueKey<bool> > transient_key;

        Implementation(const RepositoryIDParams & e) :
            env(e.environment()),
            name(e.name()),
            version("0", VersionSpecOptions()),
            repo(e.repository()),
            transient_key(new LiteralMetadataValueKey<bool>("transient", "transient", mkt_internal, true))
        {
        }
    };
}

RepositoryID::RepositoryID(const RepositoryIDParams & entry) :
    PrivateImplementationPattern<RepositoryID>(new Implementation<RepositoryID>(entry)),
    _imp(PrivateImplementationPattern<RepositoryID>::_imp)
{
    add_metadata_key(_imp->transient_key);
}

RepositoryID::~RepositoryID()
{
}

void
RepositoryID::need_keys_added() const
{
}

void
RepositoryID::need_masks_added() const
{
}

const std::string
RepositoryID::canonical_form(const PackageIDCanonicalForm f) const
{
    switch (f)
    {
        case idcf_full:
            return stringify(_imp->name) + "-" + stringify(_imp->version) +
                "::" + stringify(_imp->repo->name());

        case idcf_no_version:
            return stringify(_imp->name) + "::" + stringify(_imp->repo->name());

        case idcf_version:
            return stringify(_imp->version);

        case idcf_no_name:
            return stringify(_imp->version) + "::" + stringify(_imp->repo->name());

        case last_idcf:
            break;
    }

    throw InternalError(PALUDIS_HERE, "Bad PackageIDCanonicalForm");
}

PackageDepSpec
RepositoryID::uniquely_identifying_spec() const
{
    return parse_user_package_dep_spec("=" + stringify(name()) + "-" + stringify(version()) +
            + "::" + stringify(repository()->name()),
            _imp->env, UserPackageDepSpecOptions());
}

const QualifiedPackageName
RepositoryID::name() const
{
    return _imp->name;
}

const VersionSpec
RepositoryID::version() const
{
    return _imp->version;
}

const std::tr1::shared_ptr<const Repository>
RepositoryID::repository() const
{
    return _imp->repo->shared_from_this();
}

bool
RepositoryID::supports_action(const SupportsActionTestBase &) const
{
    return false;
}

void
RepositoryID::perform_action(Action & a) const
{
    throw ActionFailedError("Unsupported action: " + a.simple_name());
}

std::tr1::shared_ptr<const Set<std::string> >
RepositoryID::breaks_portage() const
{
    return make_shared_ptr(new Set<std::string>);
}

bool
RepositoryID::arbitrary_less_than_comparison(const PackageID &) const
{
    return false;
}

std::size_t
RepositoryID::extra_hash_value() const
{
    return 0;
}

const std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
RepositoryID::contains_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
RepositoryID::contained_in_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >
RepositoryID::fs_location_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<FSEntry> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<bool> >
RepositoryID::transient_key() const
{
    return _imp->transient_key;
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >
RepositoryID::virtual_for_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > >();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
RepositoryID::keywords_key() const
{
    return std::tr1::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
RepositoryID::provide_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
RepositoryID::dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
RepositoryID::build_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
RepositoryID::run_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
RepositoryID::post_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
RepositoryID::suggested_dependencies_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
RepositoryID::short_description_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::string> >
RepositoryID::long_description_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
RepositoryID::fetches_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >();
}

const std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
RepositoryID::homepage_key() const
{
    return std::tr1::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >
RepositoryID::contents_key() const
{
    return std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Contents> > >();
}

const std::tr1::shared_ptr<const MetadataTimeKey>
RepositoryID::installed_time_key() const
{
    return std::tr1::shared_ptr<const MetadataTimeKey>();
}

const std::tr1::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
RepositoryID::from_repositories_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >
RepositoryID::choices_key() const
{
    return make_null_shared_ptr();
}

const std::tr1::shared_ptr<const MetadataValueKey<SlotName> >
RepositoryID::slot_key() const
{
    return make_null_shared_ptr();
}

template class PrivateImplementationPattern<RepositoryID>;

