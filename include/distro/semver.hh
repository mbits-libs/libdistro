// Copyright 2021 midnightBITS
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace distro {
	class semver {
	public:
		class comp {
			std::variant<unsigned, std::string> value;

		public:
			comp() = default;
			comp(unsigned val) : value{val} {}
			comp(std::string const& val) : value{val} {}
			comp(std::string&& val) : value{std::move(val)} {}
			std::string to_string() const;
			bool operator<(comp const& rhs) const;
			bool operator==(comp const& rhs) const;
			static comp from_string(std::string_view comp);
		};

		unsigned major;
		unsigned minor;
		unsigned patch;
		std::vector<comp> prerelease;
		std::vector<std::string> meta;

		std::string to_string() const;
		bool operator<(semver const& rhs) const;
		bool operator==(semver const& rhs) const;
		static std::optional<semver> from_string(std::string_view view);
	};
}  // namespace distro
