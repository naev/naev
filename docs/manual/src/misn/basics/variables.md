# Mission Variables

Mission variables allow storing arbitrary variables in save files.
Unlike the `mem` per-mission/event [memory model](./memory.md), these are per-player and can be read and written by any Lua code.
The API is available as part of the [`var` module](https://naev.org/api/modules/var.html).

The core of the `var` module is three functions:

* `var.peek( varname )`: allows obtaining the value of a mission variable called `varname`.
  If it does not exist it returns `nil`.
* `var.push( varname, value )`: creates a new mission variable `varname` or overwrites an existing mission variable `varname` if it exists with the value `value`.
  Note that not all data types are supported, but many are.
* `var.pop( varname)`: removes a mission variable.

It is common to use mission variables to store outcomes in mission strings that affect other missions or events.
Since they can also be read by any Lua code, they are useful in `<cond>` header statements too.

Supported variable types are `number`, `boolean`, `string`, and `time`.
If you want to pass systems and other data, you have to pass it via untranslated name `:nameRaw()` and then use the corresponding `.get()` function to convert it to the corresponding type again.
