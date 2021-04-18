// Copyright 2021 midnightBITS
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#include <distro/semver.hh>

#include <algorithm>
#include <charconv>
#include <iterator>

#include <distro/regex.hh>

namespace distro {
	static auto const semvercomp_pattern = std::regex{
	    "^0|[1-9]\\d*$", std::regex::ECMAScript | std::regex::optimize};
	static auto const semver_pattern = std::regex{
	    "^(0|[1-9]\\d*)\\.(0|[1-9]\\d*)\\.(0|[1-9]\\d*)(?:-((?:0|[1-9]\\d*|\\d*"
	    "[a-"
	    "zA-Z-][0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-zA-Z-]*))*"
	    "))?("
	    "?:\\+([0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?$",
	    std::regex::ECMAScript | std::regex::optimize};

	inline unsigned to_unsigned(std::string_view in) {
		unsigned result{};
		(void)std::from_chars(in.data(), in.data() + in.size(), result);
		return result;
	}

	std::string semver::comp::to_string() const {
		if (std::holds_alternative<unsigned>(value))
			return std::to_string(std::get<unsigned>(value));
		return std::get<std::string>(value);
	}

	bool semver::comp::operator<(comp const& rhs) const {
		if (std::holds_alternative<unsigned>(value)) {
			if (std::holds_alternative<unsigned>(rhs.value))
				return std::get<unsigned>(value) <
				       std::get<unsigned>(rhs.value);
			return true;  // strings > numbers
		}

		if (std::holds_alternative<unsigned>(rhs.value))
			return false;  // strings > numbers

		return std::get<std::string>(value) < std::get<std::string>(rhs.value);
	}

	bool semver::comp::operator==(comp const& rhs) const {
		return value == rhs.value;
	}

	semver::comp semver::comp::from_string(std::string_view view) {
		if (std::regex_match(view.begin(), view.end(), semvercomp_pattern))
			return to_unsigned(view);
		return std::string{view.data(), view.size()};
	}

	std::string semver::to_string() const {
		auto result = std::to_string(major);
		result.push_back('.');
		result.append(std::to_string(minor));
		result.push_back('.');
		result.append(std::to_string(patch));

		char pre = '-';
		for (auto const& seg : prerelease) {
			result.push_back(pre);
			result.append(seg.to_string());
			pre = '.';
		}

		pre = '+';
		for (auto const& seg : meta) {
			result.push_back(pre);
			result.append(seg);
			pre = '.';
		}
		return result;
	}

	bool semver::operator<(semver const& rhs) const {
		if (major != rhs.major) return major < rhs.major;
		if (minor != rhs.minor) return minor < rhs.minor;
		if (patch != rhs.patch) return patch < rhs.patch;

		auto const lhsLen = prerelease.size();
		auto const rhsLen = rhs.prerelease.size();
		auto const minlen = std::min(lhsLen, rhsLen);

		for (size_t index = 0; index < minlen; ++index) {
			auto const& lhsItem = prerelease.at(index);
			auto const& rhsItem = rhs.prerelease.at(index);
			if (lhsItem < rhsItem) return true;
			if (rhsItem < lhsItem) return false;
			// continue...
		}
		// the one with shorter prerelease is greater...
		return rhsLen < lhsLen;
	}

	bool semver::operator==(semver const& rhs) const {
		if ((major != rhs.major) || (minor != rhs.minor) ||
		    (patch != rhs.patch) ||
		    (prerelease.size() != rhs.prerelease.size()))
			return false;

		auto lhsIt = prerelease.begin();
		auto rhsIt = rhs.prerelease.begin();
		auto end = prerelease.end();
		for (; lhsIt != end; ++lhsIt, ++rhsIt) {
			auto const& lhsItem = *lhsIt;
			auto const& rhsItem = *rhsIt;
			if (!(lhsItem == rhsItem)) return false;
		}
		return true;
	}

	template <typename Capture>
	std::optional<std::vector<std::string>> splitIdentifiers(
	    Capture const& capture) {
		if (!capture.matched) {
			// valid identifier state -- missing...
			return std::vector<std::string>{};
		}
		auto chunks = to_view(capture);
		size_t prev = 0, curr = 0, end = chunks.size();
		std::optional<std::vector<std::string>> result{
		    std::vector<std::string>{}};

		while (curr != end) {
			auto const c = static_cast<unsigned char>(chunks[curr]);
			if (c == '.') {
				if (prev == curr)  // empty chunk
					return std::nullopt;
				if (chunks[prev] == '0' && (curr - prev) > 1)
					return std::nullopt;

				result->emplace_back(chunks.data() + prev,
				                     chunks.data() + curr);
				++curr;
				prev = curr;
				continue;
			}

			if (!std::isalnum(c) && c != '-') return std::nullopt;
			++curr;
		}
		if (prev == curr)  // empty chunk
			return std::nullopt;
		if (chunks[prev] == '0' && (curr - prev) > 1) return std::nullopt;

		result->emplace_back(chunks.data() + prev, chunks.data() + curr);
		return result;
	}

	std::optional<semver> semver::from_string(std::string_view view) {
		svmatch match{};
		if (std::regex_match(view.begin(), view.end(), match, semver_pattern)) {
			auto& major = match[1];
			auto& minor = match[2];
			auto& patch = match[3];
			auto& prerelease = match[4];
			auto& meta = match[5];

			auto prerel_strings = splitIdentifiers(prerelease);
			auto meta_strings = splitIdentifiers(meta);

			if (prerel_strings && meta_strings) {
				std::vector<comp> comps;
				comps.reserve(prerel_strings->size());

				std::transform(prerel_strings->begin(), prerel_strings->end(),
				               std::back_inserter(comps), [](auto const& str) {
					               return comp::from_string(str);
				               });

				return semver{to_unsigned(to_view(major)),
				              to_unsigned(to_view(minor)),
				              to_unsigned(to_view(patch)), std::move(comps),
				              std::move(*meta_strings)};
			}
		}
		return std::nullopt;
	}
}  // namespace distro
