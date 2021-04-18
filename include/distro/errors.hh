// Copyright 2021 midnightBITS
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#pragma once

#include <string>
#include <system_error>
#include <unordered_set>

namespace distro {
	class semver;

	struct errors {
		virtual ~errors();
		[[noreturn]] virtual void src_dir(std::error_code const& ec) const = 0;
		[[noreturn]] virtual void dst_dir(std::error_code const& ec) const = 0;
		[[noreturn]] virtual void version_missing(
		    semver const& missing) const = 0;
	};
}  // namespace distro
