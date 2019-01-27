# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
- Nothing at the moment

## [1.4.0] - 2019-1-21
### Added
- Camera previews on app window
- Number of Ableton Link peers on app window
- Names of connected monitors on app window
- Go button starts the performance, and a colored background indicates running
- Set levels on ETC Element lightboard via MIDI
- Execute a function for all linearly interpolated values of a range in a set number of beats
- Media
   - [performance](https://www.dropbox.com/s/tniv8zyec0jc3de/IMG_2768.mov?dl=0)
   - [rehearsal](https://www.dropbox.com/sh/ykcqou1ojxk7l1m/AABV4-qVop_dipSbKFXg7Obqa?dl=0)
   - [images](https://www.dropbox.com/sh/84pf8ghbpi7nld4/AACKp6Tk1vpWRHz0mNgy3Cj8a?dl=0)

## [1.3.0] - 2019-01-07
### Added
- solid_color_view draws a square with solid background color on a window
- window_manager::getWindowForMonitor(monitorName) selects fetches the window for a present monitor
- monitor_manager::excludeMonitor(monitorName) will prevent monitor from being added
- stream_manager::excludeDevice(deviceName) will prevent device from being added
- window::getMonitor()
- Defined Optoma 1080p as default monitor for video
- Defined Optoma WXGA as default monitor for light
- During recording phase, video projector is faded black while light projector is unfaded using the solid_color_view
- Flicker length can vary over time, creating a rhythm of white/black and gas clicks.
- Media
   - [performance](https://www.dropbox.com/sh/kavy5i113q0f0qa/AABDZlJ9aIg6h-DCGEtqdB5Ia?dl=0)
   - [rehearsal](https://www.dropbox.com/sh/ro03mfzacmmjoeu/AACyHA4E6OudOhF54bC11-Mha?dl=0)
   - [images](https://www.dropbox.com/sh/h96nfvk4u1yw9ih/AAAg8KZ4s6Piz8TEfTFe737Za?dl=0)

## [1.2.0] - 2018-12-18
### Added
- venue: Stone Way Wide Open Mic
- setup: projector, laptop, and webcam become optional
- [5 minute portable audio mix](https://www.dropbox.com/s/6963o05o7lv1tvq/Flicker.wav?dl=0)

## [1.1.0] - 2018-12-17
### Added
- Ableton: drier egg timer, printer, tick
- setup: optionally install all hardware ahead of time
- video
   - [performance](https://vimeo.com/307926111)
   - [improvised material](https://www.dropbox.com/sh/3ra3wx4kre13q9r/AAA2LOrc37Lagq3tcwH42_03a?dl=0)

### Fixed
- setup: ensure audience can hear audio 

## [1.0.0] - 2018-12-03
### Added
- venue: SH*T GOLD
- Ableton: hammer on metal, gas click 1 & 2, volca loop 6
- [movement vocabulary](https://www.dropbox.com/sh/tz0bht4j0wkbmnb/AADISMIJ_5A9b47Rmvd1KMtua?dl=0)
- setup: install hardware right before performance starts
- video
   - [performance](https://vimeo.com/307925301)
   - [rehearsal](https://www.dropbox.com/sh/qvt56qeqvs92wb1/AAAcmIStd5B1zFth13NdRLura?dl=0)
   - [improvised material](https://www.dropbox.com/sh/wamgz512igkkch6/AABcvAux8IHUHlR5lQB-Ld8fa?dl=0)
