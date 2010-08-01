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
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/name.hh>
#include <paludis/version_spec.hh>
#include <paludis/metadata_key.hh>
#include <paludis/literal_metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/unchoices_key.hh>
#include <paludis/user_dep_spec.hh>

using namespace paludis;
using namespace paludis::repository_repository;

namespace
{
    struct RepositoryIDBehaviours :
        Singleton<RepositoryIDBehaviours>
    {
        std::shared_ptr<Set<std::string> > behaviours_value;
        std::shared_ptr<LiteralMetadataStringSetKey> behaviours_key;

        RepositoryIDBehaviours() :
            behaviours_value(std::make_shared<Set<std::string>>()),
            behaviours_key(std::make_shared<LiteralMetadataStringSetKey>("behaviours", "behaviours", mkt_internal, behaviours_value))
        {
            behaviours_value->insert("transient");
            behaviours_value->insert("used");
        }
    };
}

namespace paludis
{
    template <>
    struct Imp<RepositoryID>
    {
        const Environment * const env;
        const QualifiedPackageName name;
        const VersionSpec version;
        const RepositoryRepository * const repo;

        const std::shared_ptr<LiteralMetadataStringSetKey> behaviours_key;

        Imp(const RepositoryIDParams & e) :
            env(e.environment()),
            name(e.name()),
            version("0", { }),
            repo(e.repository()),
            behaviours_key(RepositoryIDBehaviours::get_instance()->behaviours_key)
        {
        }
    };
}

RepositoryID::RepositoryID(const RepositoryIDParams & entry) :
    Pimp<RepositoryID>(entry),
    _imp(Pimp<RepositoryID>::_imp)
{
    add_metadata_key(_imp->behaviours_key);
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
            _imp->env, { });
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

const std::shared_ptr<const Repository>
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

std::shared_ptr<const Set<std::string> >
RepositoryID::breaks_portage() const
{
    return std::make_shared<Set<std::string>>();
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

const std::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >
RepositoryID::contains_key() const
{
    return std::shared_ptr<const MetadataCollectionKey<PackageIDSequence> >();
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >
RepositoryID::contained_in_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >();
}

const std::shared_ptr<const MetadataValueKey<FSEntry> >
RepositoryID::fs_location_key() const
{
    return std::shared_ptr<const MetadataValueKey<FSEntry> >();
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
RepositoryID::behaviours_key() const
{
    return _imp->behaviours_key;
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >
RepositoryID::virtual_for_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > >();
}

const std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >
RepositoryID::keywords_key() const
{
    return std::shared_ptr<const MetadataCollectionKey<KeywordNameSet> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >
RepositoryID::provide_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
RepositoryID::dependencies_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
RepositoryID::build_dependencies_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
RepositoryID::run_dependencies_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
RepositoryID::post_dependencies_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >
RepositoryID::suggested_dependencies_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> >();
}

const std::shared_ptr<const MetadataValueKey<std::string> >
RepositoryID::short_description_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<std::string> >
RepositoryID::long_description_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::string> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >
RepositoryID::fetches_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> >();
}

const std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >
RepositoryID::homepage_key() const
{
    return std::shared_ptr<const MetadataSpecTreeKey<SimpleURISpecTree> >();
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Contents> > >
RepositoryID::contents_key() const
{
    return std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Contents> > >();
}

const std::shared_ptr<const MetadataTimeKey>
RepositoryID::installed_time_key() const
{
    return std::shared_ptr<const MetadataTimeKey>();
}

const std::shared_ptr<const MetadataCollectionKey<Set<std::string> > >
RepositoryID::from_repositories_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > >
RepositoryID::choices_key() const
{
    return make_null_shared_ptr();
}

const std::shared_ptr<const MetadataValueKey<SlotName> >
RepositoryID::slot_key() const
{
    return make_null_shared_ptr();
}

template class Pimp<RepositoryID>;

