## Naev Lore Website

The directory here is for generating the Naev Lore Website, which consists of easily accessible Naev data for the web environment.
The data is directly generated from the original data files, such that is always up to date and easy to use.

The current repository is very much WIP.

### Goals

The goal is to not only have complete data, but also have the data ready in a format to be able to be used by the game itself for the Naev encycplopedia.

### Dependencies

* ruby and ruby development packages (`ruby-devel` on Fedora)
* libyaml development headers (`libyaml-devel` on Fedora)
* bundler (should pull in the rest of ruby dependencies)
* yq (on Arch linux it is the `go-yq` package)
* tidy
