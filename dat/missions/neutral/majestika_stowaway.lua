--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Majestika Stowaways">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <notes>
  <done_evt name="Majestika Stowaways Aboard">Triggers</done_evt>
 </notes>
</mission>
--]]
--[[
   Majestika Stowaways

   Player has to find a good home for the pair of orphans that snuck into their ship.
--]]
local neu = require "common.neutral"
local fmt = require "format"
local vn = require "vn"
local ant = require "common.antlejos"

local title = _([[Majestika Stowaways]])
local stowaway_spob, stowaway_sys = spob.get("Majestika II")

function create ()
   misn.accept()
   misn.setTitle( title )
   misn.setDesc(fmt.f(_([[You have to find a good home for a pair of orphans that snuck unto your ship when you landed on {spob} in the {sys} system.]]),
      {spob=stowaway_spob, sys=stowaway_sys}))
   misn.setReward(_([[A good life for the pair of orphans.]]))
   misn.osdCreate( title,{
      _([[Find a suitable home for the little stowaways]]),
   } )
   mem.hook_land = hook.land("land")

   mem.visited = {}
end

local function spob_goodness( s )
   local f = s:faction()
   if not s:services()["inhabited"] or s:population() < 10 or not f then
      return -1, "#r".._("Devoid of inhabitants").."#0"
   end

   local t = s:tags()
   if t.restricted or t.prison or t.ruined then
      return -1, "#r".._("Poor environment for kids").."#0"
   end

   if t.station then
      return 0, "#o".._("Stations are not very good for kids").."#0"
   end
   if t.poor or t.criminal or t.military then
      return 0, "#o".._("Inadequate enviroment for kids").."#0"
   end

   local fs = f:playerStanding()
   if fs < 0 then
      return 0, "#o".._("You are in poor local standing here").."#0"
   end

   if t.immigration or t.rich then
      return 1, "#g".._("Welcoming environment for kids").."#0"
   end

   if fs >= 20 then
      return 1, "#g".._("You are in good local standing here").."#0"
   end

   return 0
end

function land ()
   local spb = spob.cur()

   if mem.visited[ spb:nameRaw() ] then return end

   local goodness, reason = spob_goodness( spb )
   local droppedoff = false

   vn.clear()
   vn.scene()
   vn.na(fmt.f(_([[The orphans peer out your window in awe as you land on {spb}.]]),
      {spb=spb}))
   -- Special case
   if spb == spob.get("Antlejos V") and player.misnDone("Terraforming Antlejos 5") then
      local verner = ant.vn_verner()
      vn.appear( verner )
      vn.na(_([[You wind up meeting Verner at the station and offer. You explain your plight to him and ask if he would be willing to take in the orphans and give them stuff to do around the station.]]))
      vn.na(_([[Without giving given it a second thought he gladly agrees.]]))
      verner(_([["I know what it is like to be without a home. It's a terrible feeling, sort of eats you out from the inside until there's nothing left. I'll gladly take the pair in and give them an education. Got to have bright kids for a bright future of Antlejos V!"]]))
      vn.disappear( verner )
      vn.sfxVictory()
      vn.na(_([[With that settled, you bright the kids over to Verner who quickly enthralls with some sleight of hand. Seeing that they are in good hands, you now return to your ship with peace of mind.]]))
      vn.func( function ()
         mem.verner = true
      end )
      vn.jump("byebye_good")

   elseif goodness < 0 then
      vn.na(fmt.f(_([[You look around but do not think that the place will make a very good home for the kids for the following reason:
{reason}]]),
         {reason=reason}))
      vn.na(_([[You'll have to find another place to drop the orphans off.]]))
      vn.func( function()
         mem.visited[ spb:nameRaw() ] = true
      end )
      vn.done()

   elseif goodness > 0 then
      vn.na(fmt.f(_([[You look around and believe that the place will make an excellent new home for the orphans for the following reason:
   {reason}]]),
         {reason=reason}))
      vn.na(fmt.f(_([[Drop off the orphans at {spb}?]]),
         {spb=spb}))
      vn.menu{
         {_([[Drop them off]]), "dropoff"},
         {_([[Keep on looking]]), "keeplooking"},
      }

   else
      vn.na(fmt.f(_([[You look around and believe that the place will make an OKish new home for the orphans for the following reason:
   {reason}
However, it may be better to try to locate a more fitting environment for them to grow up in.]]),
         {reason=reason}))
      vn.na(fmt.f(_([[Drop off the orphans at {spb}?]]),
         {spb=spb}))
      vn.menu{
         {_([[Drop them off]]), "dropoff"},
         {_([[Keep on looking]]), "keeplooking"},
      }
   end

   vn.label("dropoff")
   vn.func( function () droppedoff = true end )
   vn.na(fmt.f(_([[Believing {spb} to be a good place for the orphans to live, you negotiate with the local authorities their handover. Eventually you manage to secure a place for them to live.]]),
      {spb=spb}))
   vn.sfxVictory()
   if goodness > 0 then
      vn.na(_([[The orphans look ecstatic at their new home and thank you for all you've done for them. Seeing that they are in good hands, you now return to your ship with peace of mind.]]))
   else
      vn.na(fmt.f(_([[The orphans look excited at their new home, a significant improvement over {spb}. It's not the ideal place for kid, but you think they will be well off at their new location.]]),
         {spb=stowaway_spob}))
   end
   vn.jump("byebye")

   vn.label("keeplooking")
   vn.na(_([[You decide to keep looking for a new home for the stowaway orphans.]]))
   vn.func( function()
      mem.visited[ spb:nameRaw() ] = true
   end )
   vn.done()

   vn.label("byebye")
   vn.func( function () droppedoff = true end )
   if goodness > 0 then
      vn.jump("byebye_good")
   end
   vn.func( function()
      mem.goodness = false
   end )
   vn.done()

   vn.label("byebye_good")
   vn.func( function () droppedoff = true end )
   vn.func( function()
      mem.goodness = true
   end )
   vn.done()

   vn.run()

   if droppedoff then
      local logtxt
      if mem.verner then
         logtxt = fmt.f(_([[You entrusted a pair of orphans that stowed away on your ship on {spb} to Verner on Antlejos V. He said he would make sure it would become their new happy home.]]),
            {spb=stowaway_spob})
      elseif mem.goodness then
         logtxt = fmt.f(_([[You found a new home at {newhome} for a pair of orphans that stowed away on your ship on {spb}. The environment seems ideal for them to grow up at.]]),
            {spb=stowaway_spob, newhome=spb})
      else
         logtxt = fmt.f(_([[You found a new home at {newhome} for a pair of orphans that stowed away on your ship on {spb}. The location is not ideal, but was the best you could do under the circumstances.]]),
            {spb=stowaway_spob, newhome=spb})
      end
      neu.addMiscLog( logtxt )
      hook.rm( mem.hook_land )
      hook.takeoff( "takeoff" )
   end
end

function takeoff ()
   -- Delayed reward
end
