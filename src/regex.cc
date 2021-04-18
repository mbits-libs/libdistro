// Copyright 2021 midnightBITS
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#include <distro/regex.hh>

using namespace std::literals;

namespace distro {
	namespace {
		std::string compile_extensions(
		    std::vector<std::string_view> const& list) {
			size_t size = list.size();
			if (size) --size;

			for (auto view : list) {
				size += view.size();
				for (auto c : view) {
					if (c == '.') ++size;
				}
			}

			std::string result{};
			result.reserve(size);

			bool first = true;
			for (auto view : list) {
				if (first)
					first = false;
				else
					result.push_back('|');
				for (auto c : view) {
					if (c == '.') result.push_back('\\');
					result.push_back(c);
				}
			}
			return result;
		}

		std::string compile_platforms(
		    std::vector<std::string_view> const& list) {
			auto size = list.size();
			if (size) --size;  // |
			for (auto view : list)
				size += view.size();

			std::string result{};
			result.reserve(size);

			bool first = true;
			for (auto view : list) {
				if (first)
					first = false;
				else
					result.push_back('|');
				result.append(view);
			}

			return result;
		}
	}  // namespace

	std::vector<std::string_view> regex::platforms() {
		return {"windows-x86_64", "windows-x86_32", "ubuntu18-x86_64",
		        "anywhere"};
	}

	std::regex build_file_matcher(
	    std::string_view package_name,
	    std::vector<std::string_view> const& platforms,
	    std::vector<std::string_view> const& extensions) {
		static constexpr auto prefix = "-([.0-9]+(?:-[.0-9a-zA-Z-]+)?)-("sv;
		static constexpr auto infix =
		    ")(?:-([a-zA-Z]+)(?:-([0-9.]+))?)?\\.(?:"sv;
		static constexpr auto suffix = ")$"sv;

		auto const pltfms = compile_platforms(platforms);
		auto const exts = compile_extensions(extensions);

		auto size = pltfms.size() + exts.size() + package_name.size() +
		            (prefix.size() + infix.size() + suffix.size()) + 1;

		std::string code{};
		code.reserve(size);
		code.push_back('^');
		code.append(package_name);
		code.append(prefix);
		code.append(pltfms);
		code.append(infix);
		code.append(exts);
		code.append(suffix);

		return std::regex{code, std::regex::ECMAScript | std::regex::optimize};
	}
}  // namespace distro
