local spacemine = require "luaspfx.spacemine"
local fmt = require "format"

local TIMER    = 5
local RANGEMIN = 300
local EXPRANGE = 500
local TRACKMIN = 3000
local TRACKMAX = 10000

local ss
function onload( o )
   ss = o:specificstats()
   TRACKMIN = ss.trackmin
   TRACKMAX = ss.trackmax
end

function descextra( _p, _o, _po )
   return fmt.f(_("Space mines detonate {timer} seconds after hostiles come within {range} {distunit} with a maximum tracking of {trackmax} {distunit} and minimum tracking of {trackmin} {distunit}. Detonations have a range of {exprange} {distunit} and damage all ships within range."), {
      distunit=naev.unit("distance"),
      timer=TIMER,
      range=RANGEMIN,
      trackmax=TRACKMAX,
      trackmin=TRACKMIN,
      exprange=EXPRANGE,
   })
end

function onshoot( p, po )
   spacemine( po:mount(p), p:vel(), p:faction(), {
      damage      = ss.damage,
      penetration = ss.penetration,
      trackmax    = ss.trackmin,
      trackmin    = ss.trackmax,
      duration    = ss.duration,
      pilot       = p,
   } )

   return true
end
