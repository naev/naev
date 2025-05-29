
### ssys remap tools
These are designed to change the star map by **changing systems position**.

 - `process_sys.sh` The main. Currently used for tests. Performs several steps:
    - Calls `sys2dot.py` to extract systems information and turn it into a dot file. At this point, the **pre-processing** occurs: some invisible edges are added to enforce desired properties.
    - Calls `neato` to compute the graph layout.
    - Calls `dot2sys.py` to extract layout information for the graph and modify ssys accordingly. At this point, the **post-processing** occurs: some geometrical transformations are applied : compute the wild space layout, enforce co-circularity of some points around Anubis BH, rotate some parts, get twin systems closer, etc.

### ssys process tools

These are designed to manage the **internal geometry of systems**, that might get affected by the position changes. (because autojumps move when the systems move)

 - `sys_freeze.py` : fixes the auto jumps position and mark them as previously auto.
 - `sys_relax.py` : rotates a frozen system for minimizing orientation stretch.
 - `sys_unfreeze.sh` : as the name suggests.
 - `sys_empty.py` : gives the list of systems that can be considered empty.

### ssys lib

 - `geometry.py` as the name suggests. Most is really classical:
    - vec for (2d-)vectors. Support for every usual composition with float. '+' and '-' are what you expect, '*' is what you expect for floats and vectors (dot product). '/' is what you expect for floats. See the source for more functionalities.
    - transformations. Obtained by **dividing** vectors: `v1/v2` is the transformation that turns `v2` into `v1`. As expected: `k1.v2*(v1/v2) = k1.v1`. So multiplying vectors with transf. gives the result of the transformation applied to the vector. Notably, when `v1` and `v2` are normalized, `v1/v2` is the rotation that turns `v2` into `v1`.
    - bb for bounding box. use += to enlarge the bb with a new element, and `in` to test if inside. Nothing fancy.
 - `ssys.py` everything else.

