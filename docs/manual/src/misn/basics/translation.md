# Translation Support

Naev supports translation through [codeberg Weblate](https://translate.codeberg.org/projects/naev/naev).
However, in order for translations to be used you have to mark strings as translatable.
This is done with a [gettext](https://www.gnu.org/software/gettext/) compatible interface.
In particular, the following functions are provided:

* `_()`: This function takes a string, marks it as translatable, and returns the translated version.
* `N_()`: This function takes a string, marks it as translatable, however, it returns the *untranslated* version of the string.
* `n_()`: Takes two strings related to a number quantity and return the translated version that matches the number quantity.
  This is because some languages translate number quantities differently.
  For example "1 apple", but "2 apple**s**".
* `p_()`: This function takes two strings, the first is a context string, and the second is the string to translate.
  It returns the translated string.
  This allows to disambiguate same strings based on context such as `p_( "main menu", "Close" )` and `p_( "some guy", "Close" )`.
  In this case `"Close"` can be translated differently based on the context strings.

In general, you want to use `_()` and `n_()` to envelop all strings that are being shown to the player, which will allow for translations to work without extra effort.
For example, when defining a new mission you want to translate all the strings as shown below:

```lua
misn.setTitle( _("My Mission") )
misn.setDesc( _("You have been asked to do lots of fancy stuff for a very fancy individual. How fancy!") )
misn.setReward( _("Lots of good stuff!") )
```

Note that `_()` and friends all assume that you are inputting strings in English.

It is important to note that strings not shown to the player, e.g., strings representing faction names or ship names, do not need to be translated!
So when adding a pilot you can just use directly the correct strings for the ship and faction (e.g., `"Hyena"` and `"Mercenary"`):

```lua
pilot.add( "Hyena", "Mercenary", nil, _("Cool Dude") )
```

Note that the name (`Cool Dude` in this case) does have to be translated!

For plurals, you have to use `n_()` given that not all languages pluralize like in English.
For example, if you want to indicate how many pirates are left, you could do something like:

```lua
player.msg(string.format(n_( "%d pirate left!", "%d pirates left!", left ), left ))
```

The above example says how many pirates are left based on the value of the variable `left`.
In the case there is a single pirate left, the singular form should be used in English, which is the first parameter.
For other cases, the plural form is used.
The value of the variable `left` determines which is used based on translated language.
Although the example above uses `string.format` to display the number value for illustrative purposes, it is recommended to format text with the `format` library explained below.
