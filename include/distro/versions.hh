// Copyright 2021 midnightBITS
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#pragma once

#include <map>
#include <vector>

#include <distro/errors.hh>
#include <distro/package.hh>

namespace distro {
	using StringSet = std::unordered_set<std::string>;

	class versions {
	public:
		using Map = std::map<semver, std::vector<package>>;
		using iterator = Map::iterator;

		class comp_list {
		public:
			std::vector<fs::path> get_archives();
			void debug_print(std::ostream&);

		private:
			friend class versions;
			comp_list(versions* parent, iterator selected, StringSet&& list)
			    : parent_{parent}
			    , selected_{selected}
			    , list_{std::move(list)} {}
			versions* parent_;
			iterator selected_;
			StringSet list_;
		};

		static std::vector<fs::path> get_archives(
		    fs::path const& srcdir,
		    StringSet const& architectures,
		    std::optional<semver>& requested,
		    std::regex const& file_matcher,
		    bool debug,
		    std::ostream& debug_out,
		    errors& log);
		static versions read_packages(fs::path const& srcdir,
		                              StringSet const& architectures,
		                              std::regex const& matcher,
		                              errors const& log);
		iterator find_selected(std::optional<semver>& requested,
		                       errors const& log);
		comp_list components(iterator const& selected);

		auto begin() const noexcept { return items.begin(); }
		auto end() const noexcept { return items.end(); }
		auto rend() const noexcept { return items.rend(); }
		auto empty() const noexcept { return items.empty(); }

	private:
		Map items;
	};
}  // namespace distro