# Graphics

Naev ships can be either added as [sprite sheets](./graphics/2d.md), or, as done usually, from [3D models using the GLTF format](./graphics/3d.md).
It is recommended to use the GLTF format when possible, as it allows the lighting to change in-game and also makes the animation look much smoother.

## Defining the Graphics

Graphics used by a ship are defined in the ship definition using the `gfx` tag, in combination with the `base_type` tag.
For example, the Llama ship graphics are defined as follows
```xml
 <base_type>Llama</base_type>
 <gfx size="47">llama.gltf</gfx>
```
Here, the `base_type` indicates that this ship is a Llama, and can be accessed from Lua with `ship:baseType()`.
It is also used to define the path to the graphics.

Next, the `gfx` tag specifies the exact file indicating the graphics.
This is different depending on whether you are using 2D or 3D graphics.
An overview is shown below:

| Approach | Data location | Notes |
| -------- | ------------- | ----- |
| 3D | `gfx/ship3d/BASETYPE/GFXNAME` | Requires setting `size` attribute. |
| 2D | `gfx/ship/BASETYPE/GFXNAME` | File extension is automatically search for if not specified. |

### Defining 3D Graphics

In the case of 3D, you want to specify the GLTF file path, which will be searched for in `gfx/ship3d/BASETYPE/GFXNAME`.
So in the case of `llama.gltf` with base type of `Llama`, it will search for in `gfx/ship3d/Llama/llama.gltf` and error if it is not found.
The `size` tag specifies how many pixels the big it will be scaled in-game.
See [graphics 3d](./graphics/3d.md) for more details.

### Defining 2D Graphics

In the case of 2D, you have to prepare several spritesheets:
1. **Ship Spritesheet:** represents the ship when it is not accelerating
2. **Engine Spritesheet:** represents the ship when it is accelerating
3. **Comm Image:** used when the player communicates with the ship

An example of a sprite sheet is shown below.

![Llama with engine glows.](./legacy_images/llama/llama.webp)<br/>
*Example of the ship graphics for the "Llama".
Starting from top-left position, and going right first before going down, the ship rotates counter-clockwise and starts facing right.
A black background has been added for visibility.*

The images will be searched with the `gfx/ship/BASETYPE/GFXNAME` prefix and extensions will be searched for as necessary.
In particular, the engine spritesheet will get `_engine` appended, and the comm image will have `_comm` appended.
See [graphics 2d](./graphics/2d.md) for more details.


### Absolute paths

You can also avoid searching in the `gfx/` paths by defining an absolute path starting with `/`.
This lets you load graphics from any location.
For example, you can use the graphics of a spob for a ship as below:

```xml
 <gfx sy="1" sx="1" comm="spob/exterior/station02" polygon="002" noengine="1" size="150">/gfx/spob/space/002</gfx>
```

Do note that, in this case, you have to specify the communication image, collision polygons, and disable the engine if it does not exist.
See [2D graphics](./graphics/2d.md) for more details at how to define the additional parameters if necessary.
