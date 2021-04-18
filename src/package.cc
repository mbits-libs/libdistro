// Copyright 2021 midnightBITS
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#include <distro/package.hh>
#include <distro/regex.hh>

using namespace std::literals;

namespace distro {
	std::optional<package> package::from_string(fs::path const& archive,
	                                            std::string_view view,
	                                            std::regex const& matcher) {
		svmatch match{};
		if (std::regex_match(view.begin(), view.end(), match, matcher)) {
			auto& version = match[1];
			auto& arch = match[2];
			auto& comp = match[3];
			auto& compver = match[4];

			auto ver = semver::from_string(to_view(version));
			if (!ver) return std::nullopt;

			package pkg{archive, std::move(*ver), arch.str()};
			pkg.archive.make_preferred();

			if (comp.matched) {
				std::optional<semver> cver{};
				if (compver.matched) {
					cver = semver::from_string(to_view(compver));
					if (!cver) return std::nullopt;
				}
				pkg.comp = component{comp.str(), std::move(cver)};
			}
			return pkg;
		}
		return std::nullopt;
	}
}  // namespace distro
