# Introduction

Naev is designed to be easily modifiable by using many plain text files, usually in the XML format, to define the data of the different components, such as [missions](./misn/overview.md), [ships](./ships/overiev.md), or [outfits](./outfits/overview.md).
They are loaded as necessary by the engine when the game is started.

Although it is possible to modify the data files directly, it is highly recommended to use the [plugin framework](./plugins/overview) so that changes are not lost on updating, and can also be easily shared with other people,
