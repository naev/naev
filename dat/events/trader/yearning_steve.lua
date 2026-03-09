--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Yearning Steve">
 <location>land</location>
 <chance>100</chance>
 <faction>Traders Society</faction>
 <cond>
   player.misnDone("Mining Vrata Intro")
 </cond>
</event>
--]]
local trader = require "common.trader"
local fmt = require "format"
local vn = require "vn"

-- These commodities have supported events to increase standing
local SPECIALS = {
   "Nebula Crystals",
   "Space Moss",
}
local function get_specials ()
   local special = {}
   for k,c in ipairs(player.fleetCargoList()) do
      local name = c.c:nameRaw()
      local t = c.c:tags()
      if inlist( SPECIALS, name) and t.mining and t.special and not player.evtDone("Mining Vrata Delivered "..name) then
         table.insert( special, c )
      end
   end
   return special
end

function create ()
   -- TODO add banter instead of only checking new stuff
   if #get_specials() <= 0 then
      evt.finish(false)
   end
   evt.npcAdd( "approach", trader.vrata_steve.name, trader.vrata_steve.portrait, _("Mining Vrata's Steve seems to be enjoying a drink.") )
   hook.enter("enter")
end

function enter ()
   evt.finish(false)
end

function approach ()
   local special = get_specials()

   vn.clear()
   vn.scene()
   local steve = vn.newCharacter( trader.vn_vrata_steve() )
   vn.transition()

   vn.na(_([[You find Steve enjoying a drink at the bar.]]))
   steve(fmt.f(_([["Hey {player}, what are you up to?"]]),
      {player=player.name()}))
   vn.label("menu")
   vn.menu( function ()
      local opts = { {_("Leave."), "01_leave"} }
      for k,s in ipairs(special) do
         table.insert( opts, 1, {
            fmt.f(_("Show him {commodity}"), {commodity=s.c}),
            s
         })
      end
      return opts
   end, function ( val )
      if type(val)=='string' then
         return vn.jump(val)
      end
      vn.jump( val.c:nameRaw() )
   end )

   vn.label("Nebula Crystals")
   vn.na(_([[You show some of the Nebula Crystals you collected to Steve, and give a short explanation of how you acquired them. He seems to take a deep look at them.]]))
   steve(_([["Wow! You found these in the Nebula? They look really hard."
He taps on the crystals.
"I'll take a small sample so we can study it at the Mining Vrata. Exciting!"]]))
   vn.sfxBingo()
   vn.func( function ()
      naev.eventStart("Mining Vrata Delivered Nebula Crystals")
   end )
   vn.jump("deliver_done")

   vn.label("Space Moss")
   vn.na(_([[You show some of the Space Moss you you collected to Steve, and give him an explanation of your travels until you found the Space Moss. He seems very interested in them.]]))
   steve(_([["Wait, there's wild bioships just chilling and eating space moss out in Fertile Crescent? There must be so much left to explore! I'll show a small sample to our researchers. Who knows what they'll find out of it!"]]))
   vn.sfxBingo()
   vn.func( function ()
      naev.eventStart("Mining Vrata Delivered Space Moss")
   end )
   vn.jump("deliver_done")

   vn.label("deliver_done")
   vn.na( function ()
      special = get_specials()
      local fct = faction.get("Traders Society")
      fct:hit( 50 )
      return fmt.f("#g".._([[{fct} maximum reputation limit increased to {amount}.]]).."#0", {
         fct    = fct,
         amount = trader.reputation_max(),
      })
   end )
   steve(_([["Anything else?"]]))
   vn.jump("menu")

   vn.label("01_leave")
   steve(_([["See ya around! Will to drill!"]]))
   vn.run()
end
