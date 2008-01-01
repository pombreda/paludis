/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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

#include <paludis/repositories/e/ebuild.hh>
#include <paludis/repositories/e/ebuild_id.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/package_dep_spec.hh>
#include <paludis/repositories/e/pipe_command_handler.hh>

#include <paludis/util/system.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/log.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/options.hh>
#include <paludis/util/map.hh>
#include <paludis/util/join.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/set.hh>

#include <paludis/about.hh>
#include <paludis/environment.hh>
#include <paludis/util/config_file.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>

#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

#include <list>

#include "config.h"

/** \file
 * Implementation for ebuild.hh things.
 *
 * \ingroup grpebuildinterface
 */

using namespace paludis;
using namespace paludis::erepository;

#include <paludis/repositories/e/ebuild-sr.cc>

EbuildCommand::EbuildCommand(const EbuildCommandParams & p) :
    params(p)
{
}

EbuildCommand::~EbuildCommand()
{
}

bool
EbuildCommand::success()
{
    return true;
}

bool
EbuildCommand::failure()
{
    return false;
}

bool
EbuildCommand::operator() ()
{
    Command cmd(getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis") +
            "/ebuild.bash '" + ebuild_file() + "' " + commands());

    if (params.sandbox)
        cmd.with_sandbox();

    if (params.userpriv)
        cmd.with_uid_gid(params.environment->reduced_uid(), params.environment->reduced_gid());

    using namespace tr1::placeholders;
    cmd.with_pipe_command_handler(tr1::bind(&pipe_command_handler, params.environment, params.package_id, _1));

    tr1::shared_ptr<const FSEntrySequence> syncers_dirs(params.environment->syncers_dirs());
    tr1::shared_ptr<const FSEntrySequence> bashrc_files(params.environment->bashrc_files());
    tr1::shared_ptr<const FSEntrySequence> fetchers_dirs(params.environment->fetchers_dirs());
    tr1::shared_ptr<const FSEntrySequence> hook_dirs(params.environment->hook_dirs());

    cmd = extend_command(cmd
            .with_setenv("P", stringify(params.package_id->name().package) + "-" +
                stringify(params.package_id->version().remove_revision()))
            .with_setenv("PV", stringify(params.package_id->version().remove_revision()))
            .with_setenv("PR", stringify(params.package_id->version().revision_only()))
            .with_setenv("PN", stringify(params.package_id->name().package))
            .with_setenv("PVR", stringify(params.package_id->version()))
            .with_setenv("PF", stringify(params.package_id->name().package) + "-" +
                stringify(params.package_id->version()))
            .with_setenv("CATEGORY", stringify(params.package_id->name().category))
            .with_setenv("REPOSITORY", stringify(params.package_id->repository()->name()))
            .with_setenv("FILESDIR", stringify(params.files_dir))
            .with_setenv("EAPI", stringify(params.package_id->eapi()->name))
            .with_setenv("PKGMANAGER", PALUDIS_PACKAGE "-" + stringify(PALUDIS_VERSION_MAJOR) + "." +
                stringify(PALUDIS_VERSION_MINOR) + "." +
                stringify(PALUDIS_VERSION_MICRO) + stringify(PALUDIS_VERSION_SUFFIX) +
                (std::string(PALUDIS_SUBVERSION_REVISION).empty() ?
                 std::string("") : "-r" + std::string(PALUDIS_SUBVERSION_REVISION)))
            .with_setenv("PALUDIS_TMPDIR", stringify(params.builddir))
            .with_setenv("PALUDIS_CONFIG_DIR", SYSCONFDIR "/paludis/")
            .with_setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "))
            .with_setenv("PALUDIS_HOOK_DIRS", join(hook_dirs->begin(), hook_dirs->end(), " "))
            .with_setenv("PALUDIS_FETCHERS_DIRS", join(fetchers_dirs->begin(), fetchers_dirs->end(), " "))
            .with_setenv("PALUDIS_SYNCERS_DIRS", join(syncers_dirs->begin(), syncers_dirs->end(), " "))
            .with_setenv("PALUDIS_COMMAND", params.environment->paludis_command())
            .with_setenv("PALUDIS_REDUCED_GID", stringify(params.environment->reduced_gid()))
            .with_setenv("PALUDIS_REDUCED_UID", stringify(params.environment->reduced_uid()))
            .with_setenv("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
            .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
            .with_setenv("PALUDIS_UTILITY_PATH_SUFFIXES",
                    params.package_id->eapi()->supported->ebuild_options->utility_path_suffixes)
            .with_setenv("PALUDIS_EBUILD_MODULE_SUFFIXES",
                    params.package_id->eapi()->supported->ebuild_options->ebuild_module_suffixes)
            .with_setenv("PALUDIS_NON_EMPTY_VARIABLES",
                    params.package_id->eapi()->supported->ebuild_options->non_empty_variables)
            .with_setenv("PALUDIS_DIRECTORY_VARIABLES",
                    params.package_id->eapi()->supported->ebuild_options->directory_variables)
            .with_setenv("PALUDIS_EBUILD_MUST_NOT_SET_VARIABLES",
                    params.package_id->eapi()->supported->ebuild_options->ebuild_must_not_set_variables)
            .with_setenv("PALUDIS_SAVE_VARIABLES",
                    params.package_id->eapi()->supported->ebuild_options->save_variables)
            .with_setenv("PALUDIS_SAVE_BASE_VARIABLES",
                    params.package_id->eapi()->supported->ebuild_options->save_base_variables)
            .with_setenv("PALUDIS_SAVE_UNMODIFIABLE_VARIABLES",
                    params.package_id->eapi()->supported->ebuild_options->save_unmodifiable_variables)
            .with_setenv("PALUDIS_DIRECTORY_IF_EXISTS_VARIABLES",
                    params.package_id->eapi()->supported->ebuild_options->directory_if_exists_variables)
            .with_setenv("PALUDIS_SOURCE_MERGED_VARIABLES",
                    params.package_id->eapi()->supported->ebuild_options->source_merged_variables)
            .with_setenv("PALUDIS_MUST_NOT_CHANGE_VARIABLES",
                    params.package_id->eapi()->supported->ebuild_options->must_not_change_variables)
            .with_setenv("PALUDIS_RDEPEND_DEFAULTS_TO_DEPEND",
                    params.package_id->eapi()->supported->ebuild_options->rdepend_defaults_to_depend ? "yes" : "")
            .with_setenv("PALUDIS_PIPE_COMMANDS_SUPPORTED", "yes")
            )
        .with_setenv("SLOT", "")
        .with_setenv("PALUDIS_PROFILE_DIR", "")
        .with_setenv("PALUDIS_PROFILE_DIRS", "");

    if (! params.package_id->eapi()->supported->ebuild_environment_variables->env_kv.empty())
        cmd.with_setenv(params.package_id->eapi()->supported->ebuild_environment_variables->env_kv, kernel_version());
    if (! params.package_id->eapi()->supported->ebuild_environment_variables->env_portdir.empty())
        cmd.with_setenv(params.package_id->eapi()->supported->ebuild_environment_variables->env_portdir, stringify(params.portdir));
    if (! params.package_id->eapi()->supported->ebuild_environment_variables->env_distdir.empty())
        cmd.with_setenv(params.package_id->eapi()->supported->ebuild_environment_variables->env_distdir, stringify(params.distdir));

    if (params.package_id->eapi()->supported->ebuild_options->support_eclasses)
        cmd
            .with_setenv("ECLASSDIR", stringify(*params.eclassdirs->begin()))
            .with_setenv("ECLASSDIRS", join(params.eclassdirs->begin(),
                        params.eclassdirs->end(), " "));

    if (params.package_id->eapi()->supported->ebuild_options->support_exlibs)
        cmd
            .with_setenv("EXLIBSDIRS", join(params.exlibsdirs->begin(),
                        params.exlibsdirs->end(), " "));

    if (params.package_id->eapi()->supported->ebuild_options->want_portage_emulation_vars)
        cmd = add_portage_vars(cmd);

    if (do_run_command(cmd))
        return success();
    else
        return failure();
}

std::string
EbuildCommand::ebuild_file() const
{
    return stringify(params.ebuild_file);
}

Command
EbuildCommand::add_portage_vars(const Command & cmd) const
{
    return Command(cmd)
        .with_setenv("PORTAGE_ACTUAL_DISTDIR", stringify(params.distdir))
        .with_setenv("PORTAGE_BASHRC", "/dev/null")
        .with_setenv("PORTAGE_BUILDDIR", stringify(params.builddir) + "/" +
             stringify(params.package_id->name().category) + "/" +
             stringify(params.package_id->name().package) + "-" +
             stringify(params.package_id->version()))
        .with_setenv("PORTAGE_CALLER", params.environment->paludis_command())
        .with_setenv("PORTAGE_GID", "0")
        .with_setenv("PORTAGE_INST_GID", "0")
        .with_setenv("PORTAGE_INST_UID", "0")
        .with_setenv("PORTAGE_MASTER_PID", stringify(::getpid()))
        .with_setenv("PORTAGE_NICENCESS", stringify(::getpriority(PRIO_PROCESS, 0)))
        .with_setenv("PORTAGE_TMPDIR", stringify(params.builddir))
        .with_setenv("PORTAGE_TMPFS", "/dev/shm")
        .with_setenv("PORTAGE_WORKDIR_MODE", "0700");
}

bool
EbuildCommand::do_run_command(const Command & cmd)
{
    return 0 == run_command(cmd);
}

EbuildMetadataCommand::EbuildMetadataCommand(const EbuildCommandParams & p) :
    EbuildCommand(p)
{
}

EbuildMetadataCommand::~EbuildMetadataCommand()
{
}

std::string
EbuildMetadataCommand::commands() const
{
    return params.commands;
}

bool
EbuildMetadataCommand::failure()
{
    return EbuildCommand::failure();
}

Command
EbuildMetadataCommand::extend_command(const Command & cmd)
{
    return Command(cmd)
        .with_uid_gid(params.environment->reduced_uid(), params.environment->reduced_gid())
        .with_stderr_prefix(stringify(params.package_id->name().package) + "-" +
                stringify(params.package_id->version()) + "> ");
}

namespace
{
    std::string purdy(const std::string & s)
    {
        std::list<std::string> tokens;
        tokenise_whitespace(s, std::back_inserter(tokens));
        return join(tokens.begin(), tokens.end(), " \\n ");
    }
}

bool
EbuildMetadataCommand::do_run_command(const Command & cmd)
{
    bool ok(false);
    keys.reset(new Map<std::string, std::string>);

    std::string input;
    try
    {
        Context context("When running ebuild command to generate metadata for '" + stringify(*params.package_id) + "':");

        std::stringstream prog;
        Command real_cmd(cmd);
        int exit_status(run_command(real_cmd.with_captured_stdout_stream(&prog)));
        input.assign((std::istreambuf_iterator<char>(prog)), std::istreambuf_iterator<char>());
        std::stringstream input_stream(input);
        KeyValueConfigFile f(input_stream, KeyValueConfigFileOptions() + kvcfo_disallow_continuations + kvcfo_disallow_comments
                + kvcfo_disallow_space_around_equals + kvcfo_disallow_unquoted_values + kvcfo_disallow_source
                + kvcfo_disallow_variables + kvcfo_preserve_whitespace);

        std::copy(f.begin(), f.end(), keys->inserter());
        if (0 == exit_status)
            ok = true;
    }
    catch (const Exception & e)
    {
        Log::get_instance()->message(ll_warning, lc_context, "Caught exception '" +
                stringify(e.message()) + "' (" + stringify(e.what()) +
                ") when generating cache for '" + stringify(*params.package_id) + "', input is '" + purdy(input) + "'");
    }

    if (ok)
        return true;
    else
    {
        Log::get_instance()->message(ll_warning, lc_context) << "Could not generate cache for '"
            << *params.package_id << "'";
        keys.reset(new Map<std::string, std::string>);
        keys->insert("EAPI", EAPIData::get_instance()->unknown_eapi()->name);

        return false;
    }
}

namespace
{
    std::string get(const tr1::shared_ptr<const Map<std::string, std::string> > & k, const std::string & s)
    {
        Map<std::string, std::string>::ConstIterator i(k->find(s));
        if (k->end() == i)
            return "";
        return i->second;
    }
}

void
EbuildMetadataCommand::load(const tr1::shared_ptr<const EbuildID> & id)
{
    Context context("When loading generated metadata for '" + stringify(*params.package_id) + "':");

    if (! keys)
        throw InternalError(PALUDIS_HERE, "keys is 0");

    if (! id->eapi()->supported)
    {
        Log::get_instance()->message(ll_debug, lc_context) << "ID pre-load EAPI '" << id->eapi()->name << "' not supported";
        id->set_slot(SlotName("UNKNOWN"));
        return;
    }
    else
        Log::get_instance()->message(ll_debug, lc_context) << "ID pre-load EAPI '" << id->eapi()->name << "' is supported";

    std::string s;
    if (! ((s = get(keys, id->eapi()->supported->ebuild_metadata_variables->metadata_eapi))).empty())
        id->set_eapi(s);
    else
        id->set_eapi(id->e_repository()->params().eapi_when_unspecified);

    if (! id->eapi()->supported)
    {
        Log::get_instance()->message(ll_debug, lc_context) << "ID post-load EAPI '" << id->eapi()->name << "' not supported";
        id->set_slot(SlotName("UNKNOWN"));
        return;
    }
    else
        Log::get_instance()->message(ll_debug, lc_context) << "ID post-load EAPI '" << id->eapi()->name << "' is supported";

    const EAPIEbuildMetadataVariables & m(*id->eapi()->supported->ebuild_metadata_variables);

    if (! m.metadata_description.empty())
        id->load_short_description(m.metadata_description, m.description_description, get(keys, m.metadata_description));

    if (! m.metadata_build_depend.empty())
        id->load_build_depend(m.metadata_build_depend, m.description_build_depend, get(keys, m.metadata_build_depend));

    if (! m.metadata_run_depend.empty())
        id->load_run_depend(m.metadata_run_depend, m.description_run_depend, get(keys, m.metadata_run_depend));

    if (! m.metadata_pdepend.empty())
        id->load_post_depend(m.metadata_pdepend, m.description_pdepend, get(keys, m.metadata_pdepend));

    if (! m.metadata_slot.empty())
    {
        try
        {
            Context c("When setting SLOT:");
            std::string slot(get(keys, m.metadata_slot));
            if (slot.empty())
            {
                Log::get_instance()->message(ll_qa, lc_context) << "Package '" << *id << "' set SLOT=\"\", using SLOT=\"0\" instead";
                slot = "0";
            }
            id->set_slot(SlotName(slot));
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message(ll_warning, lc_context) << "Setting SLOT for '" << *id << "' failed due to exception '"
                << e.message() << "' (" << e.what() << ")";
            id->set_slot(SlotName("0"));
        }
    }

    if (! m.metadata_src_uri.empty())
        id->load_src_uri(m.metadata_src_uri, m.description_src_uri, get(keys, m.metadata_src_uri));

    if (! m.metadata_homepage.empty())
        id->load_homepage(m.metadata_homepage, m.description_homepage, get(keys, m.metadata_homepage));

    if (! m.metadata_license.empty())
        id->load_license(m.metadata_license, m.description_license, get(keys, m.metadata_license));

    if (! m.metadata_provide.empty())
        id->load_provide(m.metadata_provide, m.description_provide, get(keys, m.metadata_provide));

    if (! m.metadata_iuse.empty())
        id->load_iuse(m.metadata_iuse, m.description_iuse, get(keys, m.metadata_iuse));

    if (! m.metadata_inherited.empty())
        id->load_inherited(m.metadata_inherited, m.description_inherited, get(keys, m.metadata_inherited));

    if (! m.metadata_keywords.empty())
        id->load_keywords(m.metadata_keywords, m.description_keywords, get(keys, m.metadata_keywords));

    if (! m.metadata_restrict.empty())
        id->load_restrict(m.metadata_restrict, m.description_restrict, get(keys, m.metadata_restrict));
}

EbuildVariableCommand::EbuildVariableCommand(const EbuildCommandParams & p,
        const std::string & var) :
    EbuildCommand(p),
    _var(var)
{
}

std::string
EbuildVariableCommand::commands() const
{
    return params.commands;
}

bool
EbuildVariableCommand::failure()
{
    return EbuildCommand::failure();
}

Command
EbuildVariableCommand::extend_command(const Command & cmd)
{
    return Command(cmd)
        .with_setenv("PALUDIS_VARIABLE", _var)
        .with_uid_gid(params.environment->reduced_uid(), params.environment->reduced_gid());
}

bool
EbuildVariableCommand::do_run_command(const Command & cmd)
{
    std::stringstream prog;
    Command real_cmd(cmd);
    int exit_status(run_command(real_cmd.with_captured_stdout_stream(&prog)));
    _result = strip_trailing_string(
            std::string((std::istreambuf_iterator<char>(prog)),
                std::istreambuf_iterator<char>()), "\n");

    return (0 == exit_status);
}

std::string
EbuildNoFetchCommand::commands() const
{
    return params.commands;
}

bool
EbuildNoFetchCommand::failure()
{
    throw FetchActionError("Fetch failed for '" + stringify(*params.package_id) + "'");
}

Command
EbuildNoFetchCommand::extend_command(const Command & cmd)
{
    Command result(Command(cmd)
            .with_setenv("A", fetch_params.a)
            .with_setenv("ROOT", fetch_params.root)
            .with_setenv("PALUDIS_PROFILE_DIR", stringify(*fetch_params.profiles->begin()))
            .with_setenv("PALUDIS_PROFILE_DIRS", join(fetch_params.profiles->begin(),
                    fetch_params.profiles->end(), " ")));

    if (! params.package_id->eapi()->supported->ebuild_environment_variables->env_aa.empty())
        result.with_setenv(params.package_id->eapi()->supported->ebuild_environment_variables->env_aa, fetch_params.aa);
    if (! params.package_id->eapi()->supported->ebuild_environment_variables->env_use.empty())
        result.with_setenv(params.package_id->eapi()->supported->ebuild_environment_variables->env_use, fetch_params.use);
    if (! params.package_id->eapi()->supported->ebuild_environment_variables->env_use_expand.empty())
        result.with_setenv(params.package_id->eapi()->supported->ebuild_environment_variables->env_use_expand, fetch_params.use_expand);

    for (Map<std::string, std::string>::ConstIterator
            i(fetch_params.expand_vars->begin()),
            j(fetch_params.expand_vars->end()) ; i != j ; ++i)
        result.with_setenv(i->first, i->second);

    return result;
}

EbuildNoFetchCommand::EbuildNoFetchCommand(const EbuildCommandParams & p,
        const EbuildNoFetchCommandParams & f) :
    EbuildCommand(p),
    fetch_params(f)
{
}

std::string
EbuildInstallCommand::commands() const
{
    return params.commands;
}

bool
EbuildInstallCommand::failure()
{
    throw InstallActionError("Install failed for '" + stringify(*params.package_id) + "'");
}

Command
EbuildInstallCommand::extend_command(const Command & cmd)
{
    std::string debug_build;
    do
    {
        switch (install_params.debug_build)
        {
            case iado_none:
                debug_build = "none";
                continue;

            case iado_split:
                debug_build = "split";
                continue;

            case iado_internal:
                debug_build = "internal";
                continue;

            case last_iado:
                break;
        }

        throw InternalError(PALUDIS_HERE, "Bad debug_build value");
    }
    while (false);

    Command result(Command(cmd)
            .with_setenv("A", install_params.a)
            .with_setenv("ROOT", install_params.root)
            .with_setenv("PALUDIS_LOADSAVEENV_DIR", stringify(install_params.loadsaveenv_dir))
            .with_setenv("PALUDIS_CONFIG_PROTECT", install_params.config_protect)
            .with_setenv("PALUDIS_CONFIG_PROTECT_MASK", install_params.config_protect_mask)
            .with_setenv("PALUDIS_EBUILD_OVERRIDE_CONFIG_PROTECT_MASK",
                install_params.disable_cfgpro ? "/" : "")
            .with_setenv("PALUDIS_DEBUG_BUILD", debug_build)
            .with_setenv("PALUDIS_PROFILE_DIR", stringify(*install_params.profiles->begin()))
            .with_setenv("PALUDIS_PROFILE_DIRS", join(install_params.profiles->begin(),
                                          install_params.profiles->end(), " "))
            .with_setenv("SLOT", stringify(install_params.slot)));

    if (! params.package_id->eapi()->supported->ebuild_environment_variables->env_aa.empty())
        result.with_setenv(params.package_id->eapi()->supported->ebuild_environment_variables->env_aa, install_params.aa);
    if (! params.package_id->eapi()->supported->ebuild_environment_variables->env_use.empty())
        result.with_setenv(params.package_id->eapi()->supported->ebuild_environment_variables->env_use, install_params.use);
    if (! params.package_id->eapi()->supported->ebuild_environment_variables->env_use_expand.empty())
        result.with_setenv(params.package_id->eapi()->supported->ebuild_environment_variables->env_use_expand, install_params.use_expand);

    for (Map<std::string, std::string>::ConstIterator
            i(install_params.expand_vars->begin()),
            j(install_params.expand_vars->end()) ; i != j ; ++i)
        result.with_setenv(i->first, i->second);

    return result;
}

EbuildInstallCommand::EbuildInstallCommand(const EbuildCommandParams & p,
        const EbuildInstallCommandParams & f) :
    EbuildCommand(p),
    install_params(f)
{
}

std::string
EbuildUninstallCommand::commands() const
{
    return params.commands;
}

std::string
EbuildUninstallCommand::ebuild_file() const
{
    return "-";
}

bool
EbuildUninstallCommand::failure()
{
    throw UninstallActionError("Uninstall failed for '" + stringify(*params.package_id) + "'");
}

Command
EbuildUninstallCommand::extend_command(const Command & cmd)
{
    Command result(Command(cmd)
            .with_setenv("ROOT", uninstall_params.root)
            .with_setenv("PALUDIS_LOADSAVEENV_DIR", stringify(uninstall_params.loadsaveenv_dir))
            .with_setenv("PALUDIS_EBUILD_OVERRIDE_CONFIG_PROTECT_MASK",
                uninstall_params.disable_cfgpro ? "/" : ""));

    if (uninstall_params.load_environment)
        result
            .with_setenv("PALUDIS_LOAD_ENVIRONMENT", stringify(*uninstall_params.load_environment))
            .with_setenv("PALUDIS_SKIP_INHERIT", "yes");

    return result;
}

EbuildUninstallCommand::EbuildUninstallCommand(const EbuildCommandParams & p,
        const EbuildUninstallCommandParams & f) :
    EbuildCommand(p),
    uninstall_params(f)
{
}

std::string
EbuildConfigCommand::commands() const
{
    return params.commands;
}

bool
EbuildConfigCommand::failure()
{
    throw ConfigActionError("Configure failed for '" + stringify(*params.package_id) + "'");
}

Command
EbuildConfigCommand::extend_command(const Command & cmd)
{
    Command result(Command(cmd)
            .with_setenv("ROOT", config_params.root));

    if (config_params.load_environment)
        result
            .with_setenv("PALUDIS_LOAD_ENVIRONMENT", stringify(*config_params.load_environment))
            .with_setenv("PALUDIS_SKIP_INHERIT", "yes");

    return result;
}

EbuildConfigCommand::EbuildConfigCommand(const EbuildCommandParams & p,
        const EbuildConfigCommandParams & f) :
    EbuildCommand(p),
    config_params(f)
{
}

WriteVDBEntryCommand::WriteVDBEntryCommand(const WriteVDBEntryParams & p) :
    params(p)
{
}

void
WriteVDBEntryCommand::operator() ()
{
    std::string ebuild_cmd(getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis") +
            "/write_vdb_entry.bash '" +
            stringify(params.output_directory) + "' '" +
            stringify(params.environment_file) + "'");

    tr1::shared_ptr<const FSEntrySequence> syncers_dirs(params.environment->syncers_dirs());
    tr1::shared_ptr<const FSEntrySequence> bashrc_files(params.environment->bashrc_files());
    tr1::shared_ptr<const FSEntrySequence> fetchers_dirs(params.environment->fetchers_dirs());
    tr1::shared_ptr<const FSEntrySequence> hook_dirs(params.environment->hook_dirs());

    Command cmd(Command(ebuild_cmd)
            .with_setenv("PKGMANAGER", PALUDIS_PACKAGE "-" + stringify(PALUDIS_VERSION_MAJOR) + "." +
                stringify(PALUDIS_VERSION_MINOR) + "." +
                stringify(PALUDIS_VERSION_MICRO) +
                (std::string(PALUDIS_SUBVERSION_REVISION).empty() ?
                 std::string("") : "-r" + std::string(PALUDIS_SUBVERSION_REVISION)))
            .with_setenv("PALUDIS_CONFIG_DIR", SYSCONFDIR "/paludis/")
            .with_setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "))
            .with_setenv("PALUDIS_HOOK_DIRS", join(hook_dirs->begin(), hook_dirs->end(), " "))
            .with_setenv("PALUDIS_FETCHERS_DIRS", join(fetchers_dirs->begin(), fetchers_dirs->end(), " "))
            .with_setenv("PALUDIS_SYNCERS_DIRS", join(syncers_dirs->begin(), syncers_dirs->end(), " "))
            .with_setenv("PALUDIS_COMMAND", params.environment->paludis_command())
            .with_setenv("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
            .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
            .with_setenv("PALUDIS_VDB_FROM_ENV_VARIABLES",
                    params.package_id->eapi()->supported->ebuild_options->vdb_from_env_variables)
            .with_setenv("PALUDIS_VDB_FROM_ENV_UNLESS_EMPTY_VARIABLES",
                    params.package_id->eapi()->supported->ebuild_options->vdb_from_env_unless_empty_variables)
            );

    if (0 != (run_command(cmd)))
        throw InstallActionError("Write VDB Entry command failed");
}

VDBPostMergeCommand::VDBPostMergeCommand(const VDBPostMergeCommandParams & p) :
    params(p)
{
}

void
VDBPostMergeCommand::operator() ()
{
    if (! getenv_with_default("PALUDIS_NO_GLOBAL_HOOKS", "").empty())
        return;

    Command cmd(getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis") +
            "/utils/wrapped_ldconfig '" + stringify(params.root) + "'");

    if (0 != (run_command(cmd)))
        throw InstallActionError("VDB Entry post merge commands failed");
}

std::string
EbuildPretendCommand::commands() const
{
    return params.commands;
}

bool
EbuildPretendCommand::failure()
{
    return false;
}

Command
EbuildPretendCommand::extend_command(const Command & cmd)
{
    Command result(Command(cmd)
            .with_stdout_prefix(stringify(params.package_id->name().package) + "-" +
                stringify(params.package_id->version()) + "> ")
            .with_stderr_prefix(stringify(params.package_id->name().package) + "-" +
                stringify(params.package_id->version()) + "> ")
            .with_prefix_discard_blank_output()
            .with_prefix_blank_lines()
            .with_setenv("ROOT", pretend_params.root)
            .with_setenv("PALUDIS_PROFILE_DIR", stringify(*pretend_params.profiles->begin()))
            .with_setenv("PALUDIS_PROFILE_DIRS", join(pretend_params.profiles->begin(),
                    pretend_params.profiles->end(), " ")));

    if (! params.package_id->eapi()->supported->ebuild_environment_variables->env_use.empty())
        result.with_setenv(params.package_id->eapi()->supported->ebuild_environment_variables->env_use, pretend_params.use);
    if (! params.package_id->eapi()->supported->ebuild_environment_variables->env_use_expand.empty())
        result.with_setenv(params.package_id->eapi()->supported->ebuild_environment_variables->env_use_expand, pretend_params.use_expand);

    for (Map<std::string, std::string>::ConstIterator
            i(pretend_params.expand_vars->begin()),
            j(pretend_params.expand_vars->end()) ; i != j ; ++i)
        result.with_setenv(i->first, i->second);

    result.with_uid_gid(params.environment->reduced_uid(), params.environment->reduced_gid());

    return result;
}

EbuildPretendCommand::EbuildPretendCommand(const EbuildCommandParams & p,
        const EbuildPretendCommandParams & f) :
    EbuildCommand(p),
    pretend_params(f)
{
}

std::string
EbuildInfoCommand::commands() const
{
    return params.commands;
}

bool
EbuildInfoCommand::failure()
{
    return false;
}

Command
EbuildInfoCommand::extend_command(const Command & cmd)
{
    std::string info_vars;
    if (info_params.info_vars.is_regular_file_or_symlink_to_regular_file())
    {
        LineConfigFile info_vars_f(info_params.info_vars, LineConfigFileOptions());
        info_vars = join(info_vars_f.begin(), info_vars_f.end(), " ");
    }

    Command result(Command(cmd)
            .with_stdout_prefix("        ")
            .with_stderr_prefix("        ")
            .with_prefix_discard_blank_output()
            .with_prefix_blank_lines()
            .with_setenv("ROOT", info_params.root)
            .with_setenv("PALUDIS_INFO_VARS", info_vars)
            .with_setenv("PALUDIS_PROFILE_DIR",
                info_params.profiles->empty() ? std::string("") : stringify(*info_params.profiles->begin()))
            .with_setenv("PALUDIS_PROFILE_DIRS", join(info_params.profiles->begin(),
                    info_params.profiles->end(), " ")));

    if (! params.package_id->eapi()->supported->ebuild_environment_variables->env_use.empty())
        result.with_setenv(params.package_id->eapi()->supported->ebuild_environment_variables->env_use, info_params.use);
    if (! params.package_id->eapi()->supported->ebuild_environment_variables->env_use_expand.empty())
        result.with_setenv(params.package_id->eapi()->supported->ebuild_environment_variables->env_use_expand, info_params.use_expand);

    for (Map<std::string, std::string>::ConstIterator
            i(info_params.expand_vars->begin()),
            j(info_params.expand_vars->end()) ; i != j ; ++i)
        result.with_setenv(i->first, i->second);

    result.with_uid_gid(params.environment->reduced_uid(), params.environment->reduced_gid());

    if (info_params.load_environment)
        result
            .with_setenv("PALUDIS_LOAD_ENVIRONMENT", stringify(*info_params.load_environment))
            .with_setenv("PALUDIS_SKIP_INHERIT", "yes");

    return result;
}

EbuildInfoCommand::EbuildInfoCommand(const EbuildCommandParams & p,
        const EbuildInfoCommandParams & f) :
    EbuildCommand(p),
    info_params(f)
{
}

