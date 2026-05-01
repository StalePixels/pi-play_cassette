# nextpi-play_cassette

A tape player for ZX Spectrum (and other 'raw pulse' 8bit micro) tape images — TZX, TAP, PZX, and CSW — built on libspectrum. Adds the transport controls of a real tape player (pause, rewind to previous block, bookmarks, jump-to-block) and variable playback speed.

The `v1` branch holds an earlier cleanroom TZX-only player, halted 2023-03-14 due to lack of transport-control API at the hardware level.

`v2` is (built on libspectrum 1.6 as shipped in NextPi 1.93) solves several problems v1 had by making them someone elses problem.

This code is not that, yet. This code is me moving from an old laptop, and a place to shoved the code from other repos into as I feel/see fit.
