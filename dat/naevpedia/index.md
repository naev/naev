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

<% function map( wid, x, y )
    local m = require "luatk.map"
    return m.newMap( wid, x, y, 400, 300, {} )
end %>
<widget map/>

Test end.
