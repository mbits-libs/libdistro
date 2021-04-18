# Distro selector

> Searches a directory for versioned archives of a distributions w components.

Given a source directory, this library will look all archives, whose names have form

```
<package_name>-<semver>-<platform>[-<component>].<ext>
```

After filtering the unwanted architectures, the fullest list of components is created out of available versions. Then, starting from either the newest version or one specifically requested, a list of archive files providing the components is gathered for the calling application to extract.

For instance, let's say the distributions for `my-awesome-app` has archives for four components: main, `comp1`, `comp2` and `doc`.

If there are main and doc archives for version `15.2.10-beta` and newest component 1 is prepared for `15.0.0-alpha` (on 64 bit Windows) and for `15.0.23-beta` (on Ubuntu 18), while component 2 is prepared for `15.1.0-beta` and `15.2.0-alpha` on both platform, then this library will prepare a list consisting of:

- `my-awesome-app-15.2.10-beta-windows-x86_64.zip`
- `my-awesome-app-15.0.0-alpha-windows-x86_64-comp1.zip`
- `my-awesome-app-15.1.0-beta-windows-x86_64-comp2-1.10.zip`
- `my-awesome-app-15.2.10-beta-anywhere-doc.zip`

for Windows 64 and

- `my-awesome-app-15.2.10-beta-ubuntu18-x86_64.tar.gz`
- `my-awesome-app-15.0.23-beta-ubuntu18-x86_64-comp1.tar.gz`
- `my-awesome-app-15.1.0-beta-ubuntu18-x86_64-comp2-1.10.tar.gz`
- `my-awesome-app-15.2.10-beta-anywhere-doc.zip`

for Ubuntu 18.04 architecture.

To get that, there is a one-stop shop inside `distro::versions` class:

```c++
auto matcher = distro::build_file_matcher(
    "my-awesome-app"sv, distro::regex::platforms(),
    {"zip", "tar.gz"});

// replace with "ubuntu18-x86_64" to target this platform
auto archives = distro::versions::get_archives(
    srcdir, {"windows-x86_64", "anywhere"}, std::nullopt,
    matcher, false, std::cout, error_logger);
```

The `<semver>` can be a [SemVer](https://semver.org/) without `+<meta>` part, that is either `<major>.<minor>.<patch>` or  `<major>.<minor>.<patch>-<prerelease>`.

If the `<platform>` is created using `distro::regex::platforms()`, then recognized platforms are: `windows-x86_64`, `windows-x86_32`, `ubuntu18-x86_64` and `anywhere`, the last one for CPU-agnostic archives, such as `source` or `doc`.
