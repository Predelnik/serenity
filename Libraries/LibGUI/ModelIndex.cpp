/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <AK/String.h>
#include <LibGUI/Model.h>
#include <LibGUI/Variant.h>

namespace GUI {

Variant ModelIndex::data(ModelRole role) const
{
    if (!is_valid())
        return {};

    ASSERT(model());
    return model()->data(*this, role);
}

const LogStream& operator<<(const LogStream& stream, const ModelIndex& value)
{
    if (value.internal_data())
        return stream << String::formatted("ModelIndex({},{},{:p})", value.row(), value.column(), value.internal_data());
    return stream << String::formatted("ModelIndex({},{})", value.row(), value.column());
}

}

namespace AK {

void Formatter<GUI::ModelIndex>::format(FormatBuilder& builder, const GUI::ModelIndex& value)
{
    Formatter<StringView> formatter { *this };

    if (value.internal_data())
        formatter.format(builder, String::formatted("ModelIndex({},{},{:p})", value.row(), value.column(), value.internal_data()));
    else
        formatter.format(builder, String::formatted("ModelIndex({},{})", value.row(), value.column()));
}

}
