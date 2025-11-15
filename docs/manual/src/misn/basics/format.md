# Formatting Text

An important part of displaying information to the player is formatting text.
While `string.format` exists, it is not very good for translations, as the Lua version can not change the order of parameters unlike C.
For this purpose, we have prepared the `format` library, which is much more intuitive and powerful than `string.format`.
A small example is shown below:

```lua
local fmt = require "format"

function create ()
   -- ...
   local spb, sys = spob.getS( "Caladan" )
   local desc = fmt.f( _("Take this cheese to {spb} ({sys}), {name}."),
         { spb=spb, sys=sys, name=player.name() } )
   misn.setDesc( desc )
end
```

Let us break down this example.
First, we include the library as `fmt`.
This is the recommended way of including it.
Afterwards, we run `fmt.f` which is the main formatting function.
This takes two parameters: a string to be formatted, and a table of values to format with.
The string contains substrings of the form `"{foo}"`, that is, a variable name surrounded by `{` and `}`.
Each of these substrings is replaced by the corresponding field in the table passed as the second parameter, which are converted to strings.
So, in this case, `{spb}` gets replaced by the value of `table.spb` which in this case is the variable `spb` that corresponds to the Spob of `Caladan`.
This gets converted to a string, which in this case is the translated name of the planet.
If any of the substrings are missing and not found in the table, it will raise an error.

There are additional useful functions in the `format` library.
In particular the following:

* `format.number`: Converts a non-negative integer into a human-readable number as a string. Gets rounded to the nearest integer.
* `format.credits`: Displays a credit value with the credit symbol ¤.
* `format.reward`: Used for displaying mission rewards.
* `format.tonnes`: Used to convert tonne values to strings.
* `format.list`: Displays a list of values with commas and the word "and".
  For example `fmt.list{"one", "two", "three"}` returns `"one, two, and three"`.
* `format.humanize`: Converts a number string to a human-readable rough string such as `"1.5 billion"`.

More details can be found in the [generated documentation](https://naev.org/api/modules/format.html).
