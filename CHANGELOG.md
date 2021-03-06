# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.4.1] - 2020-09-17
### Changed

- Update code and examples for compatibility with Mbed OS 6.3.

## [0.4.0] - 2020-02-15
### Added

- Added compatibility with Mbed OS 5.15.

### Fixed

- Added fix that prevents magnetometer hangs in the continuous mode.
- Adjust internal code according mbed-os coding style.

## [0.3.0] - 2019-10-13
### Added

- Added optional parameter `start` to `LSM303DLHCAccelerometer::init` and
  `LSM303DLHCMagnetometer::init` methods that allows to specify initial sensor state.

## [0.2.1] - 2019-09-15
### Fixed

- Move some float constants outside of class definition to prevent compilation warning.

### Minor

- Checked compatibility with Mbed OS 5.13.

## [0.2.0] - 2019-02-14
### Added

- Added greentea tests
- FIFO mode for an accelerometer

## [0.1.0] - 2018-07-29
### Added

- Added basic API for an accelerometer
- Added basic API for a magnetometer
- Added examples of the accelerometer/magnetometer usage
