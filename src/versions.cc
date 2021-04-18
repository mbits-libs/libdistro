// Copyright 2021 midnightBITS
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#include <distro/versions.hh>

static std::string path_from(std::filesystem::path const& path) {
	auto const u8gen = path.generic_u8string();
#ifdef __cpp_lib_char8_t
	return {reinterpret_cast<char const*>(u8gen.data()), u8gen.size()};
#else
	return u8gen;
#endif
}

namespace distro {
	std::vector<fs::path> versions::get_archives(
	    fs::path const& srcdir,
	    StringSet const& architectures,
	    std::optional<semver>& requested,
	    std::regex const& file_matcher,
	    bool debug,
	    std::ostream& debug_out,
	    errors& log) {
		auto self = read_packages(srcdir, architectures, file_matcher, log);
		if (self.empty()) {
			debug_out << "No versions found\n";
			std::exit(0);
		}

		auto selected = self.find_selected(requested, log);
		auto comps = self.components(selected);
		auto archives = comps.get_archives();

		if (debug) comps.debug_print(debug_out);

		return archives;
	}

	versions versions::read_packages(fs::path const& srcdir,
	                                 StringSet const& architectures,
	                                 std::regex const& matcher,
	                                 errors const& log) {
		std::error_code ec;
		fs::directory_iterator dirent{srcdir, ec};
		if (ec) log.src_dir(ec);

		versions versions{};
		for (auto const& entry : dirent) {
			if (entry.is_directory(ec)) continue;
			if (ec) continue;

			auto pkg = package::from_string(
			    entry.path(), path_from(entry.path().filename()), matcher);

			if (!pkg) continue;
			if (!architectures.empty() &&
			    architectures.find(pkg->arch) == architectures.end())
				continue;

			versions.items[pkg->version].emplace_back(std::move(*pkg));
		}

		return versions;
	}

	versions::iterator versions::find_selected(std::optional<semver>& requested,
	                                           errors const& log) {
		// PRE: !items.empty()
		if (!requested) requested = std::prev(items.end())->first;

		auto selected = items.find(*requested);
		if (selected == items.end() && requested->prerelease.empty()) {
			auto cur = items.rbegin();
			auto end = items.rend();
			for (; cur != end; ++cur) {
				auto const& ver = cur->first;
				if (ver.major == requested->major &&
				    ver.minor == requested->minor &&
				    ver.patch == requested->patch) {
					selected = std::prev(cur.base());
					break;
				}
			}
		}

		if (selected == items.end()) log.version_missing(*requested);

		return selected;
	}

	versions::comp_list versions::components(iterator const& selected) {
		StringSet comps;
		for (auto& [ver, pkgs] : *this) {
			for (auto const& pkg : pkgs) {
				if (pkg.comp) comps.insert(pkg.comp->name);
			}
			if (ver == selected->first) break;
		}

		return {this, selected, std::move(comps)};
	}

	std::vector<fs::path> versions::comp_list::get_archives() {
		std::vector<fs::path> archives{};
		archives.reserve(list_.size());

		for (auto& pkg : selected_->second) {
			pkg.selected = true;
			archives.push_back(pkg.archive);
			if (!pkg.comp) continue;

			auto where = list_.find(pkg.comp->name);
			if (where != list_.end()) list_.erase(where);
		}

		if (!list_.empty()) {
			auto revCurr = std::reverse_iterator{selected_};
			auto revEnd = parent_->rend();
			for (; !list_.empty() && revCurr != revEnd; ++revCurr) {
				for (auto& pkg : revCurr->second) {
					if (!pkg.comp) continue;

					auto where = list_.find(pkg.comp->name);
					if (where == list_.end()) continue;

					pkg.selected = true;
					archives.push_back(pkg.archive);
					list_.erase(where);
				}
			}
		}

		return archives;
	}

	void versions::comp_list::debug_print(std::ostream& out) {
		if (!list_.empty()) {
			out << "Possibly-missing component(s):";
			for (auto const& comp : list_) {
				out << ' ' << comp;
			}
			out << "\n\n";
		}

		for (auto const& [ver, const_pkgs] : *parent_) {
			auto pkgs = const_pkgs;
			std::sort(std::begin(pkgs), std::end(pkgs),
			          [](auto const& lhs, auto const& rhs) {
				          if (!lhs.comp) return !!rhs.comp;
				          if (!rhs.comp) return false;
				          if (lhs.comp->name != rhs.comp->name)
					          return lhs.comp->name < rhs.comp->name;
				          return lhs.comp->version < rhs.comp->version;
			          });

			auto const any_selected = [&] {
				for (auto const& pkg : pkgs) {
					if (pkg.selected) return true;
				}
				return false;
			}();

			auto const all_selected = ver == selected_->first;

			if (all_selected)
				out << "\x1b[96m";
			else if (any_selected)
				out << "\x1b[36m";
			else
				out << "\x1b[90m";

			out << ver.to_string() << "\x1b[0m";

			bool diff_arch = false;
			for (size_t i = 1; i < pkgs.size(); ++i) {
				if (pkgs[i].arch != pkgs[i - 1].arch) {
					diff_arch = true;
					break;
				}
			}

			auto pre = ':';
			for (auto const& pkg : pkgs) {
				out << pre << ' ';
				pre = ',';

				if (pkg.selected)
					out << (all_selected ? "\x1b[92m" : "\x1b[32m");
				if (pkg.comp) {
					out << pkg.comp->name;
					if (pkg.comp->version)
						out << '-' << pkg.comp->version->to_string();
				} else {
					out << "main";
				}
				if (pkg.selected) out << "\x1b[0m";

				if (diff_arch) out << " (" << pkg.arch << ')';
			}
			out << '\n';
		}

		out << '\n';
	}
}  // namespace distro
