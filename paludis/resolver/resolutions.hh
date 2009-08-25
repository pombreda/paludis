/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLUTIONS_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_RESOLUTIONS_HH 1

#include <paludis/resolver/resolutions-fwd.hh>
#include <paludis/resolver/resolution-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE Resolutions :
            private PrivateImplementationPattern<Resolutions>
        {
            public:
                Resolutions();
                ~Resolutions();

                void append(const std::tr1::shared_ptr<const Resolution> &);

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag,
                        const std::tr1::shared_ptr<const Resolution> > ConstIterator;
                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<resolver::Resolutions>;
#endif
}

#endif
