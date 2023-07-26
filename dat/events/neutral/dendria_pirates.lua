--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Dendria Pirates">
 <unique/>
 <location>load</location>
 <chance>100</chance>
</event>
--]]
--[[
   Small blockade of pirates in the Dendria system that persist until
   eliminated
--]]
local vn = require 'vn'
local fmt = require 'format'
local ccomm = require "common.comm"
local lmisn = require "lmisn"
local pir = require "common.pirate"

local mainsys = system.get("Dendria")
local piratename = _("Green Goatee")
local cost = 300e3

function create ()
   if not evt.claim{ mainsys } then
      warn(fmt.f(_("Unable to claim {sys} system!"),{sys=mainsys}))
      return
   end

   hook.enter( "enter" )
end

local pirboss
local baddies = {}
function enter ()
   if system.cur()~=mainsys then
      return
   end

   -- Get rid of spawns
   pilot.clear()
   pilot.toggleSpawn(false)

   -- Spawn pirates
   local function spawn_pirate( shipname, pos, boss )
      local p = pilot.add( shipname, "Marauder", pos + vec2.newP( 100*rnd.rnd(), rnd.angle() ), nil, {ai="guard"} )
      p:setHostile(true)
      local m = p:memory()
      m.bribe_no = _([["You ain't payin' yer wait outta this one!"]])
      m.refuel_no = _([["Do I look like a fuel station?"]])
      if boss then
         p:setLeader( boss )
      end
      hook.pilot( p, "exploded", "pir_gone" )
      hook.pilot( p, "jump",     "pir_gone" )
      hook.pilot( p, "land",     "pir_gone" )
      table.insert( baddies, p )
      return p
   end

   local mainpos = vec2.new( 14e3, 3e3 )
   pirboss = spawn_pirate( "Pirate Kestrel", mainpos )
   pirboss:rename(piratename)
   pirboss:setVisplayer(true)
   spawn_pirate( "Pirate Admonisher", mainpos, pirboss )
   spawn_pirate( "Pirate Phalanx", mainpos, pirboss )
   spawn_pirate( "Pirate Ancestor", mainpos, pirboss )
   spawn_pirate( "Pirate Shark", mainpos, pirboss )

   hook.timer( 5, "pirate_spam" )
end

function pirate_spam ()
   if not pirboss or not pirboss:exists() then
      return
   end

   pirboss:broadcast(_("This system is now mine! Prepare to be dominated!"))
   player.autonavReset( 5 )

   hook.timer( 3, "pirate_check" )
end

-- Check to see if player is spotted
local pirhook
function pirate_check ()
   local spotted = false
   for k,p in ipairs(pirboss:getVisible()) do
      if p:withPlayer() then
         spotted = true
         break
      end
   end

   if spotted then
      pirboss:hailPlayer()
      pirhook = hook.pilot( pirboss, "hail", "pirate_hail" )
      return
   end

   hook.timer( 3, "pirate_check" )
end

function pirate_hail ()
   vn.reset()
   vn.scene()
   local p = ccomm.newCharacter( vn, pirboss )
   vn.transition()
   vn.na(fmt.f(_([[You open a communication channel with {pirate}.]]),{pirate=piratename}))
   p(fmt.f(_([["Whoa, whoa, where do you think you're going champ? This system is now dominated by the mighty {pirate}!"]]),
      {pirate=piratename}))
   p(fmt.f(_([["That means you gotta pay a fee to be here. Ain't cheap either. {creds}. Take it or become space debris."]]),
      {creds=fmt.credits(cost)}))

   vn.label("menu")
   vn.menu( function ()
      local opts = {
         {fmt.f(_([[Pay {creds} (You have {money}).]]),{creds=fmt.credits(cost), money=fmt.credits(player.credits())}), "pay"},
         {_([["Isn't this Empire space?"]]), "space"},
         {fmt.f(_([["'{pirate}' doesn't sound very intimidating."]]),{pirate=piratename}), "taunt"},
         {_([[Close channel.]]), "leave"},
      }
      if pir.maxClanStanding() > 0 or pir.isPirateShip(player.pilot()) then
         table.insert( opts, 1, {_([["Hey, I'm a pirate too!"]]), "pirate"} )
      end
      return opts
   end )

   vn.label("pay")
   vn.func( function ()
      if player.credits() < cost then
         vn.jump("broke")
         return
      end
   end )
   vn.na(_([[You wire them the extortion fee.]]))
   p(_([["Pleasure doing business with you! Feel free to stay as long as you want in my system, but if you leave the system, you'll have to pay to get back in."]]))
   vn.func( function ()
      player.pay( -cost )
      for k,b in ipairs(baddies) do
         b:setBribed(true)
         b:taskClear()
      end
   end )
   vn.done()

   vn.label("broke")
   vn.na(_([[You have insufficient funds pay the extortion fee.]]))
   vn.jump("menu")

   vn.label("space")
   p(fmt.f(_([["More like was. It's {pirate} space now baby!"]]),
      {pirate=piratename}))
   vn.jump("menu")

   vn.label("pirate")
   p(_([["Consider this the family discount."
You can hear them laugh at their own joke.]]))
   vn.na(_([[Looks like someone needs an ass-whoppin'. You close the communication channel and warm your weapon systems. Time to get to pirate business!]]))
   vn.jump("fight")

   vn.label("taunt")
   vn.na(fmt.f(_([[The communication channel abruptly closes and your sensors promptly warn you that {pirate} is warming up their weapons systems.]]),
      {pirate=piratename}))
   vn.jump("fight")

   vn.label("leave")
   vn.na(fmt.f(_([[You abruptly close the communication panel and your sensors promptly warn you that {pirate} is warming up their weapons systems.]]),
      {pirate=piratename}))

   vn.label("fight")
   vn.func( function ()
      for k,b in ipairs(baddies) do
         b:setHostile(true)
      end
   end )

   vn.run()

   player.commClose()
   hook.rm( pirhook )
end

function pir_gone ()
   -- Check if done
   local left = 0
   for k,p in ipairs(baddies) do
      if p:exists() then
         left = left+1
      end
   end
   if left > 0 then
      player.msg(fmt.f(n_(
         [[{left} marauder left to clear the blockade.]],
         [[{left} marauders left to clear the blockade.]], left),{left=left}))
      return
   end

   lmisn.sfxVictory()
   player.msg(_("You have cleared the marauder blockade!"))
   evt.finish(true)
end
