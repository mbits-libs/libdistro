// Copyright 2021 midnightBITS
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#pragma once

#include <distro/semver.hh>
#include <filesystem>
#include <optional>
#include <regex>
#include <string>

namespace distro {
	namespace fs = std::filesystem;

	struct package {
		struct component {
			std::string name;
			std::optional<semver> version;
		};
		fs::path archive{};
		semver version{};
		std::string arch;
		std::optional<component> comp{};
		bool selected{false};

		static std::optional<package> from_string(fs::path const& archive,
		                                          std::string_view view,
		                                          std::regex const& matcher);
	};
}  // namespace distro
