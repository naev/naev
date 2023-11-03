--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Minerva Station Epilogue 1">
 <unique/>
 <location>enter</location>
 <chance>30</chance>
 <cond>player.misnDone("Minerva Finale 2")</cond>
 <priority>11</priority>
 <notes>
  <campaign>Minerva</campaign>
  <done_misn name="Minerva Finale 2" />
 </notes>
</event>
--]]
--[[
   Small post-campaign event that gives the player an accessory reward
--]]
local fmt = require "format"
local vn = require 'vn'

function create ()
   if not naev.claimTest( system.cur(), true ) then evt.finish(false) end

   hook.timer( 15+20*rnd.rnd(), "enter" )
end

function enter ()
   -- Retest, just in case
   if not naev.claimTest( system.cur(), true ) then evt.finish(false) end

   -- Make sure not in combat
   local incombat = false
   local pp = player.pilot()
   local t = pp:target()
   if t and t:areEnemies(pp) then
      incombat = true
   end
   -- Same logic as snd/music.lua
   local enemies = pp:getEnemies( 5e3 )
   for k,v in ipairs(enemies) do
      local tgt = v:target()
      if tgt and tgt:withPlayer() then
         incombat = true
      end
   end
   -- Delay test a bit
   if incombat then
      hook.timer( 15+0*rnd.rnd(), "enter" )
      return
   end

   local reward = outfit.get("Cyborg Feather")

   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(_([[You are sitting at your command chair when you suddenly hear a horrible scraping noise throughout the ship. Fearing the worst, you unholster your weapon and carefully go investigate.]]))
   vn.na(_([[Eventually you find it seems to be coming out of a cleaning robot that is tidying your ship. You find something jammed in it, it looks quite odd. You fiddle a bit with it until you are able to pull it out.]]))
   vn.na(_([[It looks like a feather? A quite fancy one at that with a metal core? Not sure what to make of it, you pocked the strange cyborg feather as you don't think Kex will need it back.]]))
   if not player.evtDone("Minerva Station Epilogue 2") then
      vn.na(_([[Maybe this is a sign that you should go pay Kex and Maikki a visit at New Haven?]]))
   end
   vn.sfxBingo()
   vn.func( function ()
      player.outfitAdd(reward)
   end )
   vn.na(fmt.reward(reward))
   vn.run()

   evt.finish(true)
end
