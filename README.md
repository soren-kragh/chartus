# Chartus

A simple Linux command line tool to generate basic charts in SVG or HTML format.
Chartus is ideal for integration in a fully automated work flow using scripts
etc. Works well for large data sets. While some tweaking options are provided,
the automatic defaults should normally be fine. See
[examples](#built-in-examples) below.

The generated SVG files are compatible with a wide variety of software. You may
however also use the `svg2png` script (relies on `rsvg-convert`) to convert to
bitmap, which is often preferred for e-mails, messages, or if the data set, and
hence the SVG, is very large.

## Build

```sh
git clone --recurse-submodules https://github.com/soren-kragh/chartus.git
cd chartus
git checkout RELEASE_TAG        # Replace RELEASE_TAG with actual release tag.
git submodule update --init --recursive
make
```

## Install

```sh
sudo make install
```

Alternatively you can install for your user only:

```sh
make install PREFIX=$HOME/.local
```

In this case remember to add `$HOME/.local/bin` to your `PATH` variable if it is not
already there.

### Installed Commands

- `chartus` — The main charting tool
- `svg2png` — Converts SVG output to PNG (uses `rsvg-convert`)

### Completely Self-Documenting

```sh
chartus -h
svg2png -h
```
See full [User Guide](#user-guide) (based on `chartus -T`).

## Uninstall

```sh
sudo make uninstall
```

# Built-In Examples

TBD

# User Guide

TBD
