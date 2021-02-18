# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [2.2.1] - 2021-02-18

### Fixed

- Upload packages for ubuntu focal and groovy.
- Revise the readme.

## [2.2.0] - 2020-11-12

### Added

- New vvplot option `-U` for plotting a velocity field.
- New vvplot option `--load-field` for plotting side-prcessed fields.

### Changed

- Replace `libmatheval` processor with Lua.
- Build (almost) all third-party libraries statically.
- Move CI from Bitbucket to the GitHub.
- Replace `googletest` testing framework with `cppunit`.

### Fixed

- Segmentation fault in vvplot due to stack-use-after-scope bug.

## [2.1.3] - 2020-02-26

### Fixed

- Fix the condition for using an inverse matrix when solving SLAE.
- Fix signs in Newton equation.
- Correct accounting of friction forces.

## [2.1.2] - 2020-01-09

### Added

- Support for negative spring damping coefficient.
- New vvplot `--ref-xy` option values: `bx`, `by`.

### Fixed

- Enhance vvencode video quality.

## [2.1.1] - 2019-02-03

### Fixed

- Mistake in external torque formula.

## [2.1.0] - 2019-01-22

### Fixed

- Correct formulae for bodies of zero area.

### Added

- Set external torque in holder.

## [2.0.0] - 2018-10-08

### Changed

- Introduce cmake-based build.
- Enable bitbucket-ci.
  - `googletest` unit tests.
  - `pytest` integration tests.
  - Publish deb packages.
- Include man pages.
- Rewrtite vvcompose in `C++`.
  - Built-in lua interpreter.
  - Built-in bodies generation.
- Rewrite vvxtract in `C++`.
  - Handle both results and stepdata.
- Rewrite vvplot in `C++`.
  - Save intermediate tar archives.
