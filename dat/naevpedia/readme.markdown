## Naevpedia (or Holo-Archives as known in-game)

Welcome to the Naevpedia formatting document! This document uses the `.markdown` extension to avoid being sucked into the Naev build system.

The Naevpedia is a WIP introduced for 0.12.0 and is meant to be a complete in-game wiki that updates as the player progresses through the game.
Contributions are highly welcome!


### Syntax Basics

Naevpedia uses [cmark](https://github.com/commonmark/cmark) to process and thus adheres to the [CommonMark](https://commonmark.org/) specification. Note that it doesn't use github extensions or any other extensions.

Files that are added to `dat/naevpedia/` will automatically be picked up if they have a `.md` extension.
Other files, such as this one, will be ignored.

Processing of files is done in a few stages:
1. First the YAML header is extracted.
1. Then lines are individually extracted and translated, excluding pure Lua lines. Make sure to write one sentence per line to minimize translation efforts!.
1. Afterwards, the text is run through a Lua Parser, which allows conditional expressions or dynamic markdown generation.
1. Finally, the resulting text from the Lua Parser stage is treated as markdown and processed to create the Naevpedia page.

#### YAML Header

The YAML header is denoted at the top with:
```
---
title: "Example Title"
---
```
and gets appended to the meta-data of the file. Currently used tags are:

1. **title**: represents the title of the file as shown in the naevpedia.
1. **priority**: affects the order in which it appears. Defaults to 5 and lower means higher up along the list. If tied priority, the tilte is used for sorting.
1. **cond**: contains a conditional Lua statement indicating when the page should be visible. By default pages will be visible unless cond is specified and evaluates to `false`.

An example would be
```
---
title: "Admonisher"
cond: "return ship.get([[Admonisher]]):known()"
---
```
which would make the page appears as "Admonisher" only if the condition is met, which in this case consists of the ship Admonisher being known to the player.


#### Translating

Translations are done on a line-by-line basis after extracting the YAML and before processing the Lua and Markdown.
For this purpose, it is best to break up text by lines.
Each line will be translated separately, but when it gets parsed by Markdown, it will become part of the same block and will not look like it was cut up.
This helps minimize the number of changes necessary when updating translations.


#### Lua Parser

It is possible to use Lua in the markdown file.
There are two different expressions.
The inline expression allows injecting small text directly into the file.
These chunks begin with `<%=` and end with `%>`.
For example, to get the description of a ship you could do `<%= ship.get([[Admonisher]]):description() %>`.
This will output the entire result of the command at the location before processing the file with the markdown parser.

You can also do conditional statements similarly, but without the `=`.
So it would simply be `<% ... %>`. For example, to show a block of markdown only when a condition is met you could do:
```
<% if faction.get([[Proteron]]):known() then %>
The Proteron are not very kind.
<% else %>
You don't know about them!
<% end %>
```
In the above case, whether or not the `Proteron` faction is known will affect what text gets generated. You can do arbitrary Lua expressions in this way, including loops and defining variables.


#### Markdown Parser

Finally, the text is parsed as Markdown using the [CommonMark](https://commonmark.org/) specification.
Things such as enumerated lists and headers are supported.
Although it does not look as good as a normal html render of markdown yet, most things should be supported.


#### Widgets

The Naevpedia also supports custom and standard luatk widgets via the Lua Parser.
In particular, you can write `<widget funcname />` pseudo-html code which will then run the global Lua function `funcname` and use it to generate a widget.
For example, you can do the following to generate a text widget.

```
<%
local luatk = require "luatk"
function textwgt( w )
    return luatk.newText( nil, 0, 0, w, nil, "Some text." )
end
%>
<widget textwgt />
```

An important note is that negative `x` locations will align the widget on the right and have it "float".
