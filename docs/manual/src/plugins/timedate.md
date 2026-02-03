# Time and Dates

It is possible to change the time and date system of the engine.
This is done by changing some [constants](./constants.md), and also modifying the auxiliary `timedate.lua` file.

The engine supports specifying time as sets of three components: Major, Minor, and Increment, such that Major > Minor > Increment.
To give an example, the base Naev scenario uses Cycles, Periods, and Seconds, where there are 10,000 periods in a cyle, and 5,000 seconds in a period.
Additionally, a number of increments increases per real-time second in game.
All these features are controlled by setting constants.

## Constants

The main constants are:
* `TIMEDATE_MINOR_IN_MAJOR`: Controls how many Minor units there are in a Major one. Has to be an integer.
* `TIMEDATE_INCREMENT_IN_MINOR`: Controls how many Increment units there are in a Minor one. Has to be an integer.
* `TIMEDATE_INCREMENTS_PER_SECOND`: Controls how many Increment units pass per real gameplay second. This can be floating point number.
* `TIMEDATE_HYPERSPACE_INCREMENTS`: Amount of Increment units that pass by default when the player jumps through hyperspace.
* `TIMEDATE_LAND_INCREMENTS`: Amount of Increment units that pass by default when the player lands on a space object.

It is possible to, for example, have the game use something similar to the Gregorian calendar by using the folowing setting:
```lua
   TIMEDATE_MINOR_IN_MAJOR = 365, -- Days in a year
   TIMEDATE_INCREMENT_IN_MINOR = 24*60*60, -- Seconds in a day
   TIMEDATE_INCREMENTS_PER_SECOND = 0, -- Time does not pass in-game
   TIMEDATE_HYPERSPACE_INCREMENTS = 24*60*60, -- One day per jump
   TIMEDATE_LAND_INCREMENTS = 24*60*60, -- One day per landing
```

## Displaying and Loading Time

Furthermore, the `timedate.lua` file can customize the display and how time is interpreted.
It consists of two functions `from_string` and `to_string`, which control how a string is converted to a time, and how time is converted to a text string, respectively.
Note that `timedate.lua` runs in a separate sandboxed Lua environment, and you can't use any API other than the `time` module.

### `from_string`

This is mainly used when loading time settings in configuration files.
Currently it is only used with `start.toml` that defines the starting in-game time.
The function takes a string as a parameter and must return a time structer that you can build with `time.new( major, minor, increment )`.

### `to_string`

This is used to display time throughout the game.
The function should take a `time` userdata as a parameter and return a string to display as text.

### Example

Say we want to continue the example in the Constants section and display the Gregorian calendar from Years and Days.
This can be done by defining both the `from_string` and `to_string` function as explained above in a `timedate.lua` file located at the base of the plugin.
An example implementation could be:
```lua
function from_string( str )
   local year, day = str:match("^(%d+), (%d+)$")
   if year then
      return time.new( tonumber(year), tonumber(day), 0 )
   end
   error("invalid time format: " .. tostring(str))
end

local DAY_TO_MONTH = {
   [0]=0,
   31,   -- Jan
   59,   -- Feb
   90,   -- Mar
   120,  -- Apr
   151,  -- May
   181,  -- Jun
   212,  -- Jul
   243,  -- Aug
   273,  -- Sep
   304,  -- Oct
   334,  -- Nov
   365,  -- Dec
}
local function day_to_month( day )
   for i,k in ipairs(DAY_TO_MONTH) do
      if day < k then
         return i, day - DAY_TO_MONTH[i-1]
      end
   end
end
local MONTHS = {
   _("January"),
   _("February"),
   _("March"),
   _("April"),
   _("May"),
   _("June"),
   _("July"),
   _("August"),
   _("September"),
   _("October"),
   _("November"),
   _("December"),
}
function to_string( nt )
   local year, day = nt:split()
   if year == 0 then
      return string.format( p_("%d day", "%d days", day), day )
   end
   local month
   month, day = day_to_month( day )
   return string.format( _("%d %s, %d"), day, MONTHS[month], year )
end
```

Important to note that this example does not use seconds increment, but that could be added.
