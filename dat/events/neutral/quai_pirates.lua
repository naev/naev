--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Quai Pirates">
 <unique/>
 <location>load</location>
 <chance>100</chance>
</event>
--]]
--[[
   Small blockade of pirates in the Quai system that persist until
   eliminated
--]]
local vn = require 'vn'
local fmt = require 'format'
local ccomm = require "common.comm"
local lmisn = require "lmisn"
local pir = require "common.pirate"

local mainsys = system.get("Quai")
local piratename = _("Brunette Goatee")
local cost = 700e3

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

   local mainpos = vec2.new( -4e3, -5.5e3 )
   pirboss = spawn_pirate( "Pirate Zebra", mainpos )
   pirboss:rename(piratename)
   pirboss:setVisplayer(true)
   for k,s in ipairs{ "Pirate Admonisher", "Pirate Phalanx", "Pirate Ancestor", "Pirate Hyena", "Pirate Shark"} do
      spawn_pirate( s, mainpos, pirboss )
   end

   local pos1 = vec2.new( -4e3, 8e3 )
   local boss1 = spawn_pirate( "Pirate Kestrel", pos1 )
   for k,s in ipairs{ "Pirate Starbridge", "Pirate Vendetta", "Pirate Hyena", "Pirate Shark"} do
      spawn_pirate( s, pos1, boss1 )
   end

   local pos2 = vec2.new( 8.5e3, -2e3 )
   local boss2 = spawn_pirate( "Pirate Kestrel", pos2 )
   for k,s in ipairs{ "Pirate Phalanx", "Pirate Ancestor", "Pirate Ancestor", "Pirate Shark"} do
      spawn_pirate( s, pos2, boss2 )
   end

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
   for i,b in ipairs(baddies) do
      for k,p in ipairs(b:getVisible()) do
         if p:withPlayer() then
            spotted = true
            break
         end
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
   p(_([["You look far away from home, kitten."]]))
   vn.na(_([[You're not sure what they mean by kitten, but you notice something weird on their face. What is up with tha goatee?]]))
   p(fmt.f(_([["You see, you're in {pirate} territory now. Pay {creds} or we'll turn you to scrap metal and sell your parts!"]]),
      {creds=fmt.credits(cost), pirate=piratename}))

   vn.label("menu")
   vn.menu( function ()
      local opts = {
         {fmt.f(_([[Pay {creds} (You have {money}).]]),{creds=fmt.credits(cost), money=fmt.credits(player.credits())}), "pay"},
         {_([["Wait, I thought I was in Sirius space?"]]), "space"},
         {_([["Wait, did you paint the goatee on your face?"]]), "taunt"},
         {_([["Hey, I'm just passing by, I mean no trouble."]]), "trouble"},
         {_([[Close channel.]]), "leave"},
      }
      if pir.maxClanStanding() > 0 or pir.isPirateShip(player.pilot()) then
         table.insert( opts, 1, {_([["You threaten a fellow pirate?"]]), "pirate"} )
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
   p(_([["You made the right choice. Enjoy your stay but you'll have to repay on reentry!"]]))
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
   p(fmt.f(_([["Sirius? They have their heads to stuck up in their collective asses and can't deal with our raw firepower. You're in {pirate} space now!"]]),
      {pirate=piratename}))
   vn.jump("menu")

   vn.label("pirate")
   p(_([["Think of this as a mutual business agreement. Nothing wrong about business between friends?"]]))
   vn.na(_([[Looks like someone needs an ass-whoppin'. You close the communication channel and warm your weapon systems. Time to get to pirate business!]]))
   vn.jump("fight")

   vn.label("trouble")
   p(_([["You may mean no trouble, but I sure as hell do. Now pay up!"]]))
   vn.jump("menu")

   vn.label("taunt")
   p(_([[They suddenly get flustered and when they speak again they seem to forcibly lower their voice.
"Wha... what are you insinuating? That I don't live up to my name with my fabulous brunette goatee?!"]]))
   vn.na(_([[Looking closely, it's clear that it is drawn on. Seems to be something like crayon?]]))
   p:rename(_("Crayon Goatee"))
   vn.func( function ()
      pirboss:rename(_("Crayon Goatee"))
      -- Much more aggressive
      for k,b in ipairs(baddies) do
         if b:exists() then
            local m = b:memory()
            m.guarddodist = m.guarddodist * 3
         end
      end
   end )
   vn.me(_([["Brunette Goatee? More like Crayon Goatee!"]]))
   p(_([[Blood swells to their face like a ripened tomato, making the crayon goatee even more obvious.]]))
   p(_([["Get him! The bastard is not getting out alive. I want their head as my table stand!"]]))
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
