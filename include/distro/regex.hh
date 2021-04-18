// Copyright 2021 midnightBITS
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#pragma once

#include <regex>
#include <string_view>

namespace distro {
	using svmatch = std::match_results<std::string_view::const_iterator>;
	using svsub_match = std::sub_match<std::string_view::const_iterator>;

	inline std::string_view to_view(svsub_match const& match) {
		auto const* ptr = std::to_address(match.first);
		auto const* end = std::to_address(match.second);
		auto const length = static_cast<size_t>(end - ptr);
		return {ptr, length};
	}

	// will build a regex matching:
	//    <package_name>-<semver>-<platform>[-<component>].<ext>
	// where:
	//  - package_name is taken verbatim from args
	//  - semver is extremally simple regex matching more and less, than what
	//    SemVer 2.0 allows: a dot or digit, followed by digits, letter, dots
	//    and hyphens, but no pluses
	//  - platform is an alternative between items in the argument, the items
	//    themselves are taken verbatim; if you want chipset distinction, place
	//    it here, like x64 and x32 on windows may be expressed in filenames as
	//    "windows-x86_32" and "windows-x86_64".
	//  - optional component with version will be used to recognize a group of
	//    archives building a single distribution.
	//  - ext is an alternative between items in the argument, with the items's
	//    dots escaped (e.g. "zip", "tar.gz" => "zip|tar\\.gz")
	std::regex build_file_matcher(
	    std::string_view package_name,
	    std::vector<std::string_view> const& platforms,
	    std::vector<std::string_view> const& extensions);

	namespace regex {
		std::vector<std::string_view> platforms();
	}
}  // namespace distro
