# Graphics

**NOTE: This section is a bit out of date.
It is now possible to use 3D ships with GLTF files and define the trails and mount points there.
This is the preferred way to give ship graphics and will be properly documented in the future.**

Ship graphics are defined in the `<GFX>` node as a string with additional attributes like number of sprites or size also defined in the XML.
Graphics for each ship are stored in a directory found in `gfx/ship/`, where the base graphics, engine glow graphics, and comm window graphics are placed separately with specific file names.

In particular, the `GFX` string name is sensitive to underscores, and the first component up to the first underscore is used as the directory name.
As an example, with `<GFX>llama</GFX>`, the graphics would have to be put in `gfx/ship/llama/`, while for `<GFX>hyena_pirate</GFX>`, the directory would be `gfx/ship/hyena`.
The specific graphics are then searched for inside the directory with the full `GFX` string value and a specific prefix.
Assuming `GFX` is the graphics name and `DIR` is the directory name (up to first underscore in `GFX`), we get:

* `gfx/ship/DIR/GFX.webp`: ship base graphic file
* `gfx/ship/DIR/GFX_engine.webp`: ship engine glow graphics file
* `gfx/ship/DIR/GFX_comm.webp`: ship communication graphics (used in the comm window)

The base graphics are stored as a spritesheet and start facing right before spinning counter-clockwise.
The top-left sprite faces to the right, and it rotates across the row first before going down to the next row.
The background should be stored in RGBA with a transparent background.
An example is shown below:

![Llama with engine glows.](legacy_images/llama/llama.webp)<br/>
*Example of the ship graphics for the "Llama".
Starting from top-left position, and going right first before going down, the ship rotates counter-clockwise and starts facing right.
A black background has been added for visibility.*

The engine glow graphics are similar to the base graphics, but should show engine glow of the ship.
This graphic gets used instead of the normal graphic when accelerated with some interpolation to fade on and off.
An example is shown below;

![Llama with engine glows.](legacy_images/llama/llama_engine.webp)<br/>
*Example of the engine glow graphics for the "Llama".
Notice the yellow glow of the engines.
A black background has been added for visibility.*

The comm graphics should show the ship facing the player and be higher resolution.
This image will be shown in large when the player communicates with them.
An example is shown below:

![Llama with engine glows.](legacy_images/llama/llama_comm.webp)<br/>
*Example of the comm graphics for the "Llama".*

## Specifying Full Paths

It is also possible to avoid all the path logic in the `<GFX>` nodes by specifying the graphics individually using other nodes.
In particular, you can use the following nodes in the XML in place of a single `<GFX>` node to specify graphics:

* `<gfx_space>`: Indicates the full path to the base graphics (`gfx/` is prepended).
  The `sx` and `sy` attributes should be specified, or they default to 8.
* `<gfx_engine>`: Indicates the full path to the engine glow graphics (`gfx/` is prepended).
  The `sx` and `sy` attributes should be specified, or they default to 8.
* `<gfx_comm>`: Indicates the full path to the comm graphics (`gfx/` is prepended).

This gives more flexibility and allows using, for example, spob station graphics for a "ship".
