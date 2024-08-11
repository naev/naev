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
    local tgt = ship.get("Llama")
    return m.newMap( nil, 10, 0, mw-200, (mw-200) * 9 / 16, {
        binaryhighlight = function ( s )
            for k,spb in ipairs(s:spobs()) do
                if spb:known() and inlist( spb:shipsSold(), tgt ) then
                    return true
                end
            end
            return false
        end
    } )
end %>
<widget map/>

Test end.
