---
title: "Main"
priority: 1
---
![](gfx/Naev.webp)

## Welcome to the Naevpedia!

Here you can find information about different things you have encountered throughout your adventures throughout the galaxy. Below are some quick links to get you started on exploring all the information available at your fingertips!

* [Mechanics](mechanics)
* [Ships](ships)
* [Outfits](outfits)
* [History](history)
* [Characters](characters)
* [Locations](locations)

<% function map ( mw )
    local m = require "luatk.map"
    return m.newMap( nil, 10, 0, mw-200, (mw-200) * 9 / 16, {} )
end %>
<widget map/>

Test end.
