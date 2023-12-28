--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Eye of Night Mystery">
 <unique/>
 <chance>100</chance>
 <cond>
   if faction.playerStanding("Sirius") &lt; 0 then
      return false
   end
   local srs = require "common.sirius"
   if var.peek("sirius_awakening") or srs.playerIsPsychic() then
      return false
   end
   return true
 </cond>
 <location>enter</location>
 <system>Eye of Night</system>
</event>
--]]
--[[
   Small mystery revolving around the ethereal Eye of Night Station.

   Gives the player psychic awakening if they have not gotten it already.
   This should be the main way to get it as it's at the entry point of Sirius
   space and should call the player's attention.
--]]
local vn = require "vn"
local fmt = require "format"
local lmisn = require "lmisn"
local tut = require "common.tutorial"
local pilotai = require "pilotai"
local pp_shaders = require "pp_shaders"
local lg = require "love.graphics"
local der = require 'common.derelict'
local lmusic = require "lmusic"

local mainspb = spob.get("Eye of Night Station")
local mainsys = system.get("Eye of Night")
local maindiff = "Eye of Night Station"

local event_done = false

function create ()
   if not evt.claim{mainsys} then
      evt.finish(false)
      return
   end

   -- Somewhat persistent
   evt.save()

   hook.enter( "enter_checkend" )
   hook.timer( 10+5*rnd.rnd(), "intro" )
end

function enter_checkend ()
   -- If player leaves system it all disappears
   if system.cur() ~= mainsys then
      if diff.isApplied(maindiff) then
         diff.remove(maindiff)
      end
      evt.finish( event_done )
   end

   -- Start the distress signal again
   if not event_done then
      hook.timer( 10+5*rnd.rnd(), "intro" )
   end
end

function intro ()
   lmisn.sfxEerie()
   player.msg(_("You receive a distress call from a location marked on your map."))
   player.autonavReset( 5 )

   system.markerAdd( mainspb:pos(), _("Distress Call") )

   hook.timer( 1, "check_dist" )
end

local player_status, badguy_status
local did_msg = false
local badguy

local function save_pilot( p, dooutfits )
   local t = {
      pos = p:pos(),
      vel = p:vel(),
      arm = p:armour(),
      shi = p:shield(),
      ene = p:energy(),
   }
   if dooutfits then
      t.out = p:outfits()
   end
   return t
end
local function restore_pilot( p, t )
   if t==nil then
      return
   end
   p:setPos( t.pos )
   p:setVel( t.vel )
   p:setHealth( t.arm, t.shi )
   p:setEnergy( t.ene )
   if t.out then
      p:outfitsEquip( t.out )
   end
end

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
   sai(fmt.f(_([["My analysis indicates that it would be safest to ignore the distress signal and continue along our way."
{shipai} looks unreasonably nervous for a rational being made of silicon and logic modules.]]),
      {shipai=tut.ainame()}))
   vn.na(fmt.f(_([[{shipai} dematerializes leaving you once again at command of your ship.]]),
      {shipai=tut.ainame()}))

   vn.done( tut.shipai.transition )
   vn.run()

   pilotai.clear()

   local off = player.pos() - mainspb:pos()
   badguy = pilot.add( "Pirate Hyena", "Marauder", mainspb:pos() + vec2.newP( 300, off:angle() ), nil, {stealth=true, ai="guard"} )
   badguy:setHostile(true)
   badguy:setNoDisable(true)
   hook.pilot( badguy, "attacked", "pir_attacked" )
   hook.pilot( badguy, "death", "pir_dead" )
   badguy:control()
   local pp = player.pilot()
   badguy:attack( pp )

   -- Just in case store here
   player_status = save_pilot( pp )
   player_status.followers = {}
   for k,f in ipairs(pp:followers()) do
      player_status.followers[ f:id() ] = save_pilot(f)
   end
   badguy_status = save_pilot( badguy, true )
end

function pir_attacked( _p, attacker )
   if not attacker:withPlayer() then
      return
   end
   local pp = player.pilot()
   player_status = save_pilot( pp )
   player_status.followers = {}
   for k,f in ipairs(pp:followers()) do
      player_status.followers[ f:id() ] = save_pilot(f)
   end
   badguy_status = save_pilot( badguy, true )
end

function pir_dead ()
   diff.apply( maindiff )
   hook.land( "land" )
end

local shader_fadeout
function land ()
   if event_done then
      evt.finish(true)
   end

   -- Once the spob is created, landing will abort the event
   if spob.cur() ~= mainspb then
      if diff.isApplied( maindiff ) then
         diff.remove( maindiff )
         evt.finish( false )
      end
      return
   end
   player.allowSave( false ) -- In case the player quits while VN is open or something weird

   local effectstr = 0

   vn.clear()
   vn.scene()
   vn.func( function ()
      vn.setBackground( function ()
         local nw, nh = naev.gfx.dim()
         vn.setColour( {0, 0, 0, effectstr} )
         lg.rectangle("fill", 0, 0, nw, nh )
      end )
   end )
   vn.transition()
   local mbg = vn.music( der.sfx.ambient )
   -- Maybe creepy_guitar.ogg?
   local mfg = vn.music( "snd/sounds/loops/alienplanet.ogg", {volume=0} ) -- Slowly gets stronger

   -- Changes the strength of the event
   local function effect_change( str )
      local estart, eend
      vn.func( function ()
         estart = effectstr
         eend = math.max( str, effectstr )
      end )
      vn.animation( 0.5, function (progress)
         effectstr = estart*(1-progress) + eend*progress
         lmusic.setVolume( mbg.m, 1-effectstr )
         lmusic.setVolume( mfg.m, effectstr )
      end )
   end

   local bar_enter, commodity_enter, shipyard_enter, outfitter_enter, last
   local bar_drink, cardkey, bar_look, outfitter_look

   last = "hallway01"
   vn.na(_([[The station systems confirm your ship and the landing dock gates open and you navigate your ship to an empty landing pad. Looking around, you can't see any other ships, which is quite odd.]]))
   vn.na(fmt.f(_([[You expect {shipai} to butt in and tell you to leave or something like that, but your comm is silent. Furthermore, it seems like {shipai} is offline. They must be pouting somewhere aboard the ship right now.]]),
      {shipai=tut.ainame()}))
   vn.na(_([[Having come this far, you decide to enter the station and find the origin of the distress signal.]]))
   effect_change( 0.1 ) -- Start to get stronger
   vn.na(_([[You enter the main hallway of the station, what strikes you is the complete absence of people, despite the fact that that station seems to be in perfect condition. Furthermore, there are signs of recent human activity such as footprints and a dropped pen.]]))
   vn.jump("hallway01")

   vn.label("hallway01")
   vn.na( function ()
      if last=="hallway01" then
         return _([[You are at the main hallway.
What do you do?]])
      end
      return _([[You head back to the main hallway.
What do you do?]])
   end )
   vn.func( function () last = "hallway01" end )
   vn.menu( function ()
      local opts = {
         {_("Go down the hallway"), "hallway02_try"},
         {_("Return to the hangar"), "hangar"},
      }
      if not bar_enter then
         table.insert( opts, 1, {_("Go to the room on the left"), "spaceportbar"} )
      else
         table.insert( opts, 1, {_("Go to the spaceport bar"), "spaceportbar"} )
      end
      if not commodity_enter then
         table.insert( opts, 1, {_("Go to the room on the right"), "commodityexchange"} )
      else
         table.insert( opts, 1, {_("Go to the commodity exchange"), "commodityexchange"} )
      end
      return opts
   end )

   vn.label("hangar")
   effect_change( 0.2 )
   vn.na(_([[You go back to the hangar, however, it seems a bit weird. The air around you gets heavy, and you almost feel like the space around you begins to warp. You are pretty sure you are moving forward, but when you realize it, you are back where you started at the main hallway of the station.]]))
   vn.jump("hallway01")

   vn.label("commodityexchange")
   effect_change( 0.2 )
   vn.na( function ()
      if last == "commodityexchange" then
         return _([[You are at the commodity exchange depot.
What do you do?]])
      elseif commodity_enter then
         return _([[You return to the commodity exchange depot.
What do you do?]])
      end
      commodity_enter = true
      return _([[You enter what seems to be the commodity exchange depot. Goods are neatly organized throughout the room. Everything is perfectly clean, without a smudge of dust, almost as if it was cleaned seconds ago.
What do you do?]])
   end )
   vn.func( function () last = "commodityexchange" end )
   vn.menu{
      {_([[Look around]]), "commodityexchange_look"},
      {_([[Return to the main hallway]]), "hallway01"},
   }

   vn.label("commodityexchange_look")
   --vn.func( function () commodity_looked = true end )
   vn.na(_([[You look around the room. Nothing seems to be really remarkable. You find an opened drink on a table near the back, and large amounts of minerals and some assorted goods.]]))
   effect_change( 0.25 )
   vn.sfxEerie()
   vn.na(_([[Eventually, you find a ledger with logs of commodity exchanges. Hey wait, some of the dates are in the future...]]))
   vn.jump("commodityexchange")

   vn.label("spaceportbar")
   effect_change( 0.2 )
   vn.func( function ()
      if bar_enter then
         vn.jump("spaceport_bar_menu")
      end
   end )
   vn.na(_([[You enter a somewhat dimly lit room. Not wanting to walk into a trap, you first listen to see if anything is there. Other than the humming and beeps of the functional station electrical systems, you don't here anything. Eventually your eyes fully adjust to the darkness.]]))
   vn.na(_([[It seems to be the spaceport bar, with very dim cozy lights to create an intimate environment. However, given the current circumstances, it feels more sinister than cozy. You make it behind the bar, and quickly turn the lights to max, almost blinding you for a second.]]))
   vn.na(_([[While not being exactly clean, with dirt amplified by the now bright lights, nothing really seems too blatantly out of place.]]))
   vn.label("spaceport_bar_menu")
   vn.na( function ()
      if last == "spaceportbar" then
         return _([[You are at the spaceport bar.
What do you do?]])
      elseif bar_enter then
         return _([[You return to the spaceport bar.
What do you do?]])
      end
      bar_enter = true
      return _([[What do you do?]])
   end )
   vn.func( function () last = "spaceportbar" end )
   vn.menu( function ()
      local opts = {
         {_([[Look around]]), "spaceport_bar_look"},
         {_([[Return to the main hallway]]), "hallway01"},
      }
      if bar_look and not bar_drink then
         table.insert( opts, 1, {_([[Drink the cocktail mixer]]), "spaceport_bar_drink"} )
      end
      return opts
   end )

   vn.label("spaceport_bar_look")
   effect_change( 0.25 )
   vn.func( function ()
      bar_look = true
      if bar_drink then
         vn.jump("spaceport_bar_look_drink")
      end
   end )
   vn.na(_([[You begin to look around more carefully. There is a large assortment of bottles of drinks from Sirius space, many are still unopened. Behind the bar there is a cocktail mixer that is cold to the touch.]]))
   vn.na(_([[You look around the tables and found an assortment of half-empty drinks on coasters that are still wet. It's almost as if people were here until a moment ago.]]))
   vn.na(_([[You look around the rest of the bar, including the lavatories, and find nothing particularly interesting. However, for a reason you can't quite fathom, you feel a strange attraction to the cocktail mixer you found behind the bar.]]))
   vn.jump("spaceport_bar_menu")

   vn.label("spaceport_bar_look_drink")
   vn.na(_([[You once again look around the bar, including the lavatories, and find nothing particularly interesting.]]))
   vn.jump("spaceport_bar_menu")

   vn.label("spaceport_bar_drink")
   vn.func( function () bar_drink = true end )
   vn.na(_([[You approach the cocktail mixer which almost seems to draw you in. You grab an empty glass, and pour yourself a drink. A clear pale blue liquid fills the glass, with a viscosity similar to water.]]))
   vn.na(_([[You take a gulp of air, and bring the glass up to your nose to take a whiff. The almost unnoticeable slightly tangy aroma seems to enter your nostrils and resonate in your skull.]]))
   vn.na(_([[Finally, you bring the glass to your lips and take a careful small sip. The taste is almost electric, jolting your entire nervous system into action. You put the glass down and notice it is empty. Did you drink it all? ]]))
   effect_change( 0.35 )
   vn.jump("spaceport_bar_menu")

   vn.label("hallway02_try")
   vn.func( function ()
      if bar_drink then
         vn.jump("hallway02")
      end
   end )
   vn.label("hallway02_lost")
   vn.na(_([[You go further down the hallway, which seems to twist and turn oddly for a station. You find a fork and take a random direction and keep on going. You soon find another fork and take another direction. Feels oddly long for a hallway.]]))
   vn.sfxEerie()
   effect_change( 0.2 )
   vn.na(_([[You keep on going on, finding forks and trying to go forward, however, before you know it, you are back to where you started.]]))
   vn.jump("hallway01")

   vn.label("hallway02")
   vn.na(_([[You go further down the hallway, which seems to twist and turn oddly for a station. You find a fork and take a random direction and keep on going. You soon find another fork and take another direction. Feels oddly long for a hallway.]]))
   effect_change( 0.5 )
   vn.na(_([[You keep on going and eventually find yourself at the end of the hallway, with a door on your left, one on your right, and another one straight in front of you."]]))
   vn.label("hallway02_menu")
   vn.na( function ()
      if last=="hallway02" then
         return _([[You are deep down the main hallway.
What do you do?]])
      end
      return _([[You head back to the main hallway.
What do you do?]])
   end )
   vn.func( function () last = "hallway02" end )
   vn.menu( function ()
      local opts = {
         {_("Go to the room at the end of the hallway"), "controlroom"},
         {_("Go back to beginning of the hallway"), "hallway01"},
      }
      if not outfitter_enter then
         table.insert( opts, 1, {_("Go to the room on the left"), "outfitter"} )
      else
         table.insert( opts, 1, {_("Go to the outfitter"), "outfitter"} )
      end
      if not shipyard_enter then
         table.insert( opts, 1, {_("Go to the room on the right"), "shipyard"} )
      else
         table.insert( opts, 1, {_("Go to the shipyard"), "shipyard"} )
      end
      return opts
   end )

   vn.label("shipyard")
   effect_change( 0.6 )
   vn.na( function ()
      if last == "shipyard" then
         return _([[You are at the shipyard.
What do you do?]])
      elseif shipyard_enter then
         return _([[You return to the shipyard.
What do you do?]])
      end
      shipyard_enter = true
      return _([[You enter a really big room with cranes and robotic arms all over. It takes you a while to realize that it is actually a shipyard. They look very different when they are devoid of ships.
What do you do?]])
   end )
   vn.func( function () last = "shipyard" end )
   vn.menu{
      {_([[Look around]]), "shipyard_look"},
      {_([[Return to the main hallway]]), "hallway02_menu"},
   }

   vn.label("shipyard_look")
   --vn.func( function () shipyard_look = true end )
   vn.na(_([[You walk around the vast room looking at all the devices here and there. They all seem to be functional and well maintained. Although it is quite obvious that there are no complete ships, you do find some spare parts and cores. Although there are many valuables, you do not find anything that gives you any information of what happened at the station.]]))
   effect_change( 0.7 )
   vn.jump("shipyard")

   vn.label("outfitter")
   effect_change( 0.6 )
   vn.na( function ()
      if last == "outfitter" then
         return _([[You are at the outfitter.
What do you do?]])
      elseif outfitter_enter then
         return _([[You return to the outfitter.
What do you do?]])
      end
      outfitter_enter = true
      return _([[You enter a compact room, the walls are covered with racks containing all sorts of gadgets and ship parts. You can make out several ion cannons. Without people it takes you a while to realize that it is most likely the outfitter.
What do you do?]])
   end )
   vn.func( function () last = "outfitter" end )
   vn.menu( function ()
      local opts = {
         {_([[Look around]]), "outfitter_look"},
         {_([[Return to the main hallway]]), "hallway02_menu"},
      }
      if outfitter_look and not cardkey then
         table.insert( opts, 1, {_([[Reach into the box]]), "outfitter_box"} )
      end
      return opts
   end )

   vn.label("outfitter_look")
   vn.func( function ()
      if cardkey then
         vn.jump("outfitter_look_cardkey")
      end
   end )
   vn.na(_([[You begin to shuffle through racks, it seems to be full of fully usable outfits from mainly House Sirius, although some MilSpec is mixed in for good measure. Some look like they would even make a good upgrade for your ship, however, that is not what you are here for.]]))
   vn.sfxEerie()
   vn.na(_([[You continue methodologically searching around the room. You finish looking at everything and are about to give up and return to the hallway when you notice there is a black box in the middle of the room. You surely couldn't have missed it, could you?]]))
   vn.func( function () outfitter_look = true end )
   effect_change( 0.7 )
   vn.jump("outfitter")

   vn.label("outfitter_look_cardkey")
   vn.na(_([[You look around again but find nothing of interest.]]))
   vn.jump("outfitter")

   vn.label("outfitter_box")
   vn.na(_([[You almost feel as if the box is staring back at you as you look it down. Although it seems small and like you could pick it up, you feel like you have to stick your hand in it, although you cannot explain why.]]))
   vn.na(_([[As your anxiety builds up, you grab tho box and slowly stick your arm into it. It feels, weirdly fuzzy, but at the same time as if some sort of pressure was clamping down on your hand.]]))
   vn.na(_([[You get the impression that your arm is extending infinitely into the abyss when suddenly you feel something bump into your fingers. You quickly snatch it and pull it out of the box.]]))
   vn.na(_([[You gasp for breath as you realize you must have forgotten to breathe during the whole ordeal. Looking down at what you pulled out of the box, it looks like it is a cardkey to open some door.]]))
   vn.func( function () cardkey = true end )
   effect_change( 0.9 )
   vn.jump("outfitter")

   vn.label("controlroom_locked")
   vn.na(_([[You try to activate the door, but to no avail. It seems to be powered up and locked, with no way to force yourself through. Looks like you'll have to find some way to open it.]]))
   effect_change( 0.6 )
   vn.jump("hallway02_menu")

   vn.label("controlroom")
   vn.func( function ()
      if not cardkey then
         vn.jump("controlroom_locked")
      end
   end )
   effect_change( 1.0 )
   vn.na(_([[You tap the cardkey you found and the door opens with cold air billowing forth until it envelops you completely.]]))
   vn.menu{
      {_("Enter the room"), "controlroom_enter"},
      {_("Enter the room"), "controlroom_enter"},
      {_("Enter the room"), "controlroom_enter"},
   }
   vn.label("controlroom_enter")
   vn.na(_([[You enter the room, it seems quite small and is poorly lit. A light flickers in one of the corners, reflecting on a large command chair with its back to you. Wait, is there someone there?]]))
   vn.menu{
      {_("Approach the chair"), "controlroom_approach"},
      {_("Approach the chair"), "controlroom_approach"},
      {_("Approach the chair"), "controlroom_approach"},
   }
   vn.label("controlroom_approach")
   vn.na(_([[You hesitantly approach the chair, sweat dripping down your suit.]]))
   vn.na(_([[The distance between you and the chair feels like it is growing instead of shrinking as you slowly approach it.]]))
   vn.na(_([[Almost there...]]))
   vn.musicStop()
   vn.na(_([[The room grows eerily calm as the chair slowly turns around, only to find yourself staring at you. The other you grins at you and your head starts to throb.]]))

   vn.scene()
   vn.func( function ()
      vn.textbox_bg_alpha = 0
      vn.show_options = false
      vn.setBackground( function ()
         local nw, nh = naev.gfx.dim()
         vn.setColour( {1, 1, 1, 1} )
         lg.rectangle("fill", 0, 0, nw, nh )
      end )
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
      shader_fadeout._dt = 0.0
      shader_fadeout._update = function( self, dt )
         self._dt = self._dt + dt * 1/3
         self.shader:send( "u_progress", math.min( 1, self._dt ) )
      end
      shader_fadeout.shader:addPPShader("final", 99)
   end )

   vn.run()

   var.push("sirius_awakening", true)
   hook.takeoff( "takeoff" )
   hook.update("shader_update")
   event_done = true
   player.takeoff()
end

function shader_update( dt )
   shader_fadeout:_update( dt )
   if shader_fadeout._dt > 1 then
      shader_fadeout.shader:rmPPShader()
      evt.finish(true)
   end
end

function takeoff ()
   local pp = player.pilot()

   -- Restore the pilot
   restore_pilot( pp, player_status )
   player_status.followers = {}
   for k,f in ipairs(pp:followers()) do
      restore_pilot( f, player_status.followers[ f:id() ] )
   end

   -- Restore the pirate
   local pir = pilot.add( "Pirate Hyena", "Marauder", mainspb:pos(), nil, {naked=true, ai="guard"} )
   restore_pilot( pir, badguy_status )
   pir:setHostile(true)

   diff.remove( maindiff )
   player.allowSave( true )
end
