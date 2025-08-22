# apt-os4

`apt-os4` is a package manager for AmigaOS 4, inspired by Debian's `apt`. It allows users to search, install, update, and remove software packages from remote repositories using a command-line interface.

## Features

- Download and parse repository metadata (`InRelease`, `Packages` files)
- Search for packages by name or description
- Install packages (`.deb` files) and extract their contents
- Update local package lists and check for updates
- Remove installed packages
- Supports options for verbose output and ignoring SSL peer verification

## Directory Structure

- `src/` — Source code files
- `build/` — Build output (object files, binaries)
- `Makefile` — Build instructions
- `.gitignore` — Files and directories to ignore in git
- `sources.list` — List of repositories

## Build Instructions

To build the project, run:

```sh
make
```

The resulting binary will be placed in the `build` directory.

## Usage

```sh
apt install <package1> <package2> ...
apt uninstall <package1> <package2> ...
apt remove <package1> <package2> ...
apt search <package> ...
apt update
```

### Options

- `--verbose` — Enable verbose output
- `--ignorepeers` — Skip SSL peer verification
- `--help` — Show usage information

## Configuration

Edit `sources.list` to add or remove repositories. Each line should follow the format:

```
deb <repository_url> <distribution>
```

Example:

```
deb https://example.com/ repo
```

## Dependencies

- [libcurl](https://curl.se/libcurl/)
- [libarchive](https://www.libarchive.org/)
- Standard C++17 libraries

## License

This project is licensed under the GPL License.