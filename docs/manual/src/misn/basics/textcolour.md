# Colouring Text

All string printing functions in Naev accept special combinations to change the colour.
This will work whenever the string is shown to the player.
In particular, the character `#` is used for a prefix to set the colour of text in a string.
The colour is determined by the character after `#`.
In particular, the following are valid values:

| Symbol | Description |
| --- | --- |
| `#0` | Resets colour to the default value. |
| `#r` | Red colour. |
| `#g` | Green colour. |
| `#b` | Blue colour. |
| `#o` | Orange colour. |
| `#y` | Yellow colour. |
| `#w` | White colour. |
| `#p` | Purple colour. |
| `#n` | Grey colour. |
| `#F` | Colour indicating friend. |
| `#H` | Colour indicating hostile. |
| `#N` | Colour indicating neutral. |
| `#I` | Colour indicating inert. |
| `#R` | Colour indicating restricted. |

Multiple colours can be used in a string such as `"It is a #ggood#0 #rmonday#0!"`.
In this case, the word `"good"` is shown in green, and `"monday"` is shown in red.
The rest of the text will be shown in the default colour.

While it is possible to accent and emphasize text with this, it is important to not go too overboard, as it can difficult translating.
When possible, it is also best to put the colour outside the string being translated.
For example `_("#rred#0")`  should be written as `"#r".._("red").."#0"`.
