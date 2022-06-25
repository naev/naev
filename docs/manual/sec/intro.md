# Introduction

Welcome to the Naev development manual! This manual is meant to cover all aspects of Naev development. It is currently a work in progress.

## Getting Started

This document assumes you have access to the Naev data. This can be either from downloading the game directly from a distribution platform, or getting directly the [naev source code](https://github.com/naev/naev). Either way it is possible to modify the game data and change many aspects of the game.

| Operating System | Data Location |
| --- | --- |
| Linux | `/usr/share/naev/dat` |
| Mac OS X | `/Applications/Naev.app/Contents/Resources/dat` |
| Windows | TODO |

Most changes will only take place when you restart Naev, although it is possible to force Naev to reload a mission or event with `naev.missionReload` or `naev.eventReload`.
