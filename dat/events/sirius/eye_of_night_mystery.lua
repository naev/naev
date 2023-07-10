--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Eye of Night Mystery">
 <unique/>
 <chance>100</chance>
 <cond>
   if faction.playerStanding("Sirius") &lt; 0 then
      return false
   end
   --if not var.peek("sirius_awakening") then
   --   return false
   --end
   return true
 </cond>
 <location>enter</location>
 <system>Eye of Night</system>
</event>
--]]
--[[
   Small mystery revolving around the ethereal Eye of Night Station.

   Gives the player psychic awakening if they have not gotten it already.
--]]
local vn = require "vn"
local fmt = require "format"
local lmisn = require "lmisn"
local tut = require "common.tutorial"
local pilotai = require "pilotai"
local pp_shaders = require "pp_shaders"
local lg = require "love.graphics"
local der = require 'common.derelict'

local mainspb = spob.get("Eye of Night Station")
local mainsys = system.get("Eye of Night")
local maindiff = "Eye of Night Station"

-- States:
--  0: event triggered
--  1: player finished landing
mem.state = 0

function create ()
   if not evt.claim{mainsys} then
      evt.finish(false)
      return
   end

   -- Somewhat persistent
   evt.save()

   hook.land( "land" )
   hook.enter( "enter_checkend" )
   hook.timer( 10+5*rnd.rnd(), "intro" )
end

function enter_checkend ()
   -- If player leaves system it all disappears
   if system.cur() ~= mainsys then
      if diff.isApplied(maindiff) then
         diff.remove(maindiff)
      end
      evt.finish(false)
   end

   -- Start the distress signal again
   hook.timer( 10+5*rnd.rnd(), "intro" )
end

function intro ()
   lmisn.sfxEerie()
   player.msg(_("You receive a distress call from a location marked on your map."))
   player.autonavReset( 5 )

   system.markerAdd( mainspb:pos(), _("Distress Call") )

   hook.timer( 1, "check_dist" )
end

local did_msg = false
local badguy
function check_dist ()
   if mainspb:pos():dist( player.pos() ) > 5e3 then
      hook.timer( 1, "check_dist" )
      return
   end

   if did_msg then return end
   did_msg = true

   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )

   vn.na(fmt.f(_([[As you approach the distress signal, {shipai} materializes in front of you.]]),
      {shipai=tut.ainame()}))
   sai(fmt.f(_([["Hey {playername}, I see you're heading towards that ominous distress signal. You sure it's a good idea?"]]),
      {playername=player.name()}))
   vn.menu{
      {_([["Someone could be hurt and need help!"]]), "01_help"},
      {_([["Relax, what's the worst that could happen?"]]), "01_worst"},
      {_([["It could lead us to spoils!"]]), "01_treasure"},
   }

   vn.label("01_help")
   sai(_([["What if we are the ones who get hurt?"]]))
   vn.jump("01_cont")

   vn.label("01_worst")
   sai(fmt.f(_([["With my 3rd to last captain, we went to explore a distress signal and... [MEMORY PURGED]."
{shipai} seems to flicker erratically for a second.
"What was I saying?"]]),
      {shipai=tut.ainame()}))
   vn.jump("01_cont")

   vn.label("01_treasure")
   sai(_([["Statistically it is more likely for the distress signal to be a trap and for us to become the spoils than to actually find things of value."]]))
   vn.jump("01_cont")

   vn.label("01_cont")
   sai(_([["My analysis indicates that it would be safest to ignore the distress signal and continue along our way."
{shipai} looks unreasonably nervous for a rational being made of silicon and logic modules.]]))
   vn.na(_([[{shipai} dematerializes leaving you once again at command of your ship.]]))

   vn.done( tut.shipai.transition )
   vn.run()

   pilotai.clear()

   local off = player.pos() - mainspb:pos()
   badguy = pilot.add( "Pirate Hyena", "Marauder", mainspb:pos() + vec2.newP( 300, off:angle() ), nil, {stealth=true, ai="guard"} )
   badguy:setHostile(true)
   badguy:setNoDisable(true)
   hook.pilot( badguy, "discovered", "discover_pir" )
   hook.pilot( badguy, "death", "pir_dead" )
end

local playerpos, playervel, playershield, playerarmour, playerenergy, pirpos, pirvel, piroutfits
function discover_pir ()
   local pp = player.pilot()
   playerpos = pp:pos()
   playervel = pp:vel()
   playerarmour, playershield = pp:health()
   playerenergy = pp:energy()

   pirpos = badguy:pos()
   pirvel = badguy:vel()
   piroutfits = badguy:outfits()
end

function pir_dead ()
   diff.apply( maindiff )
   hook.land( "land" )
end

local shader_fadeout
function land ()
   -- Once the spob is created, landing will abort the event
   if spob.cur() ~= mainspb then
      if diff.isApplied( maindiff ) then
         diff.remove( maindiff )
         evt.finish( false )
      end
      return
   end

   local effectstr = 0

   vn.clear()
   vn.scene()
   vn.setBackground( function ()
      if effectstr > 0 then
         local nw, nh = naev.gfx.dim()
         vn.setColor( {0, 0, 0, effectstr} )
         lg.rectangle("fill", 0, 0, nw, nh )
      end
   end )
   vn.transition()
   local mbg = vn.music( der.sfx.ambient )
   -- Maybe creepy_guitar.ogg?
   local mfg = vn.music( "snd/sounds/loop/alienplanet.ogg", {volume=0} ) -- Slowly gets stronger

   -- Changes the strength of the event
   local function effect_change( str )
      vn.func( function ()
         effectstr = str
      end )
      vn.musicVolume( mbg, 1-effectstr )
      vn.musicVolume( mfg, effectstr )
   end

   vn.na(_([[The station systems confirm your ship and the landing dock gates open and you navigate your ship to an empty landing pad. Looking around, you can't see any other ships, which is quite odd.]]))
   vn.na(fmt.f(_([[You expect {shipai} to butt in and tell you to leave or something like that, but your comm is silent. Furthermore, it seems like {shipai} is offline. They must be pouting somewhere aboard the ship right now.]]),
      {shipai=tut.ainame()}))
   vn.na(_([[Having come this far, you decide to enter the station and find the origin of the distress signal.]]))
   effect_change( 0.05 ) -- Start to get stronger
   vn.na(_([[You enter the main hallway of the station, what strikes you is the complete absence of people, despite the fact that that station seems to be in perfect condition. Furthermore, there are signs of recent human activity such as footprints and a dropped pen.]]))
   vn.na(_([[What do you do?]]))
   vn.menu{
   }

   vn.scene()
   vn.setBackground( function ()
      local nw, nh = naev.gfx.dim()
      vn.setColor( {1, 1, 1, effectstr} )
      lg.rectangle("fill", 0, 0, nw, nh )
   end )
   vn.transition( "blinkout" )

   vn.func( function ()
      -- Fades in shader from white
      local fadein_pixelcode = [[
#include "lib/blur.glsl"

const float INTENSITY = 10.0;

uniform float u_progress = 0.0;

vec4 effect( sampler2D tex, vec2 texture_coords, vec2 screen_coords )
{
   float disp = INTENSITY*(0.5-distance(0.5, u_progress));
   vec4 c1 = vec4(vec3(1.0),1.0);
   vec4 c2 = blur9( tex, texture_coords, love_ScreenSize.xy, disp );
   return mix( c1, c2, u_progress );
}
   ]]
      shader_fadeout = { shader=pp_shaders.newShader( fadein_pixelcode ) }
      shader_fadeout._dt = 0
      shader_fadeout._update = function( self, dt )
         self._dt = self._dt + dt * 1/5
         self.shader:send( "u_progress", math.min( 1, self._dt ) )
      end
      shader_fadeout.shader:addPPShader("game", 99)
   end )

   vn.run()

   player.allowSave( false )
   var.push("sirius_awakening", true)
   hook.takeoff( "takeoff" )
   hook.timer( 5, "shader_cleanup" )
   player.takeoff()
end

function shader_cleanup ()
   shader_fadeout.shader:rmPPShader()
end

function takeoff ()
   local pp = player.pilot()

   -- Restore the pilot
   pp:setPos( playerpos )
   pp:setVel( playervel )
   pp:setHealth( playerarmour, playershield )
   pp:setEnergy( playerenergy )

   -- Restore the pirate
   local pir = pilot.add( "Pirate Hyena", "Marauder", pirpos, nil, {naked=true, ai="guard"} )
   pir:setVel( pirvel )
   for k,o in ipairs(piroutfits) do
      if o then
         pir:outfitAddSlot( o, k, true, true )
      end
   end
   pir:setHostile(true)

   player.allowSave( true )
   evt.finish(true)
end
