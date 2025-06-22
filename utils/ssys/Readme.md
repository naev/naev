
### ssys remap tools
These are designed to change the star map by **changing systems position**.

 - `process_ssys.sh` The main. Currently used for tests. Performs several steps:
    - Calls `ssys2dot.py` to extract systems information and turn it into a dot file. At this point, the __pre-processing__ occurs: some invisible edges are added to enforce desired properties.
    - Calls `neato` to compute the graph layout.
    - Calls `dot2graph.py` to extract layout information from the dot input and outputs the resulting graph. At this point, the __post-processing__ occurs: some geometrical transformations are applied : compute the wild space layout, enforce co-circularity of some points around Anubis BH, rotate some parts, get twin systems closer, etc.
   - Calls `ssysgraph.py -w` to apply the new graph coordinates to the `ssys/*.xml` files. `ssysgraph.py` with no option, reads to `ssys/*.xml` files and outputs a graph, while `ssysgraph.py -w` does the opposite.

### ssys process tools
These are designed to manage the **internal geometry of systems**, that might get affected by the position changes. (because autojumps move when the systems move)

 - `ssys_freeze.py` : fixes the auto jumps position and mark them as previously auto.
 - `ssys_relax.py` : rotates a frozen system for minimizing orientation stretch.
 - `ssys_unfreeze.sh` : as the name suggests.
 - `ssys_empty.py` : gives the list of systems that can be considered empty.

### ssys lib
 - `geometry.py` as the name suggests. Most is really classical:
    - vec for (2d-)vectors. Support for every usual composition with float. `+` and `-` are what you expect, `*` is what you expect for floats and vectors (dot product). `/` is what you expect for floats. See the source for more functionalities.
    - transformations. Obtained by __dividing__ vectors: `v1/v2` is the transformation that turns `v2` into `v1`. As expected: `k * v2 * (v1/v2) = k*v1`. So multiplying vectors with transf. gives the result of the transformation applied to the vector. Notably, when `v1` and `v2` are normalized (or just have the same size), `v1/v2` is the rotation that turns `v2` into `v1`.
    - lines (line intersection) / circle (circumscribed / inscribed / bounding circle).
    - bb for bounding box. use += to enlarge the bb with a new element, and `in` to test if inside. Nothing fancy.
 - `ssys.py` everything else.

### potential
 - `potential -g` generates the __gravity potential__ of the star map as a picture (black means minimum potential, and white means 0 potential (upper bound).
 - `height_map.pov` allows to display that potential in 3D. Use `gen_pot.sh` to do it all.
 - `potential -a` can __apply potential__, see `apply_pot.sh` and `apply_pot_to_ssys.sh`.

### auto-positionning system
 - `repos.sh` read current ssys info and applies reposition repeatedly. See `repos.sh -h`.
 - `reposition` reads systems position and edges in input, and outputs the list of repositionned systems. The system you want to reposition are given as arguments. See `reposition -h`.
