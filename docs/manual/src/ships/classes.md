# Ship Classes

Ships classes have an intrinsic size parameter accessible with the `ship.size()` Lua API.
This is a whole integer number from 1 to 6.

In Naev: Sea of Darkness, small ships (size 1 and 2) use small core slots and are meant to be fast and small.
Medium ships (size 3 and 4) use medium core slots and are still agile, while being able to pack more of a punch.
Large ships (size 5 and 6) are slow hulking giants with heavy slots meant to dominate.
There is always a trade-off between agility and raw power, giving all ships a useful role in the game.

Ships are also split into two categories: civilian and military.
Civilian ships are meant to focus more on utility and flexibility, while military ships focus more on combat abilities.

An overview of all the ship classes is shown below:

* **Civilian**
  * **Yacht**: very small ship often with only few crew members (size 1)
  * **Courier**: small transport ship (size 2)
  * **Freighter**: medium transport ship (size 3)
  * **Amoured Transport**: medium ship with some combat abilities (size 4)
  * **Bulk Freighter**: large transport ship (size 5)
* **Military**
  * **Small**
    * **Scout**: small support ship (size 1)
    * **Interceptor**: ultra-small attack ship (size 1)
    * **Fighter**: small attack ship (size 2)
    * **Bomber**: missile-based small attack ship (size 2)
  * **Medium**
    * **Corvette**: agile medium ship (size 3)
    * **Destroyer**: heavy-medium ship (size 4)
  * **Large**
    * **Cruiser**: large ship (size 5)
    * **Battleship**: firepower-based extremely large ship (size 6)
    * **Carrier**: fighter bay-based extremely large ship (size 6)

Note that it is also possible to give custom class names.
For example, you can have a ship be of class `Yacht`, yet show the class name as `Luxury Yacht` in-game.
The only thing that matters from the ship class is the internal size which is used in different computations.
