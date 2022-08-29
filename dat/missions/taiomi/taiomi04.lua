--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 4">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <done>Taiomi 3</done>
 <notes>
  <campaign>Taiomi</campaign>
  <tier>2</tier>
 </notes>
</mission>
--]]
--[[
   Taiomi 04

   Player has to do some resource collection / mining
]]--
local vn = require "vn"
local fmt = require "format"
local taiomi = require "common.taiomi"
local equipopt = require "equipopt"

-- luacheck: globals enter land scene00 (Hook functions passed by name)

local reward = taiomi.rewards.taiomi04
local title = _("Escaping Taiomi")
local base, basesys = spob.getS("One-Wing Goddard")
-- Asks for Therite which easiest obtained from Haven nearby
-- Closer systems only have Vixilium
local minesys = system.get("Haven")
local resource = commodity.get("Therite")
local amount = 30

--[[
   0: mission started
   1: interaction with ships on the way there
   2: obtained materials
--]]
mem.state = 0

local function do_osd ()
   misn.osdCreate( title, {
      fmt.f(_("Obtain {amount} of {resource} (have {have})"),
         {resource=fmt.tonnes_short(resource),
            amount=amount,
            have=fmt.tonnes_short(mem.brought+player.fleetCargoOwned(resource))}),
      fmt.f(_("Return to the {spobname} ({spobsys})"), {spobname=base, spobsys=basesys}),
   } )
end

function create ()
   misn.accept()
   mem.brought = 0

   -- Mission details
   misn.setTitle( title )
   misn.setDesc(fmt.f(_("You have been asked to fly to obtain {resource} to help do an experiment. {resource} can be found in the nearby {sys} system."),
      {resource=resource, sys=minesys}))
   misn.setReward( fmt.credits(reward) )
   mem.marker = misn.markerAdd( minesys )

   do_osd()

   hook.enter( "enter" )
   hook.land( "land" )
end

function enter ()
   if mem.timer then
      hook.rm( mem.timer )
      mem.timer = nil
   end
   if mem.state > 1 then
      return
   end
   local scur = system.cur()

   if mem.state == 0 and scur ~= basesys and scur ~= minesys and rnd.rnd() < 0.5 and naev.claimTest( scur, true ) then
      -- Small cutscene with the curious drones
      mem.timer = hook.timer( 3+rnd.rnd()*3, "scene00" )
   end

   if scur ~= minesys then
      return
   end

   -- Create small convoy that tries to leave
   local pos = vec2.new( 6000, 6000 ) -- Center of asteroid field
   local fct = faction.dynAdd( "Pirate", "pirminers", _("Miners"), { clear_allies=true, clear_enemies=true } )
   local l = pilot.add( "Rhino", fct, pos )
   equipopt.pirate( l )
   l:cargoAdd( resource, l:cargoFree() )
   for i=1,4 do
      local s = "Shark"
      if rnd.rnd() < 0.5 then
         s = "Vendetta"
      end
      local p = pilot.add( s, fct, pos )
      equipopt.pirate( p )
      p:setLeader( l )
   end
end

function scene00 ()
end

function land ()
   local c = spob.cur()
   if mem.state < 2 and c ~= base then
      return
   end

   vn.clear()
   vn.scene()
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )
   vn.na(_([[You board the Goddard and find Scavenger waiting for you.]]))
   s(_([["How did it go?"]]))
   if mem.toughtime then
      vn.na(_([[You explain the ordeal you went through to get the data, without sparing details of your heroic last sprint out of the laboratory complex.]]))
      s(_([["I think the human saying goes, 'all's well that end's well'."]]))
   else
      vn.na(_([[You explain how easily it was for you to go through the laboratory complex and get the data for a professional saboteur such as yourself.]]))
      s(_([["You are exceeding all expectations!"]]))
   end
   s(_([["Let me analyze the documents and finish matching the correspondences with all the collected data. I believe this should be sufficient to design something useful."]]))
   s(_([["I shall be waiting for you outside."
Scavenger backs out of the Goddard and returns to space.]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( taiomi.scavenger.transition )
   vn.run()

   player.pay( reward )
   taiomi.log.main(_("You stole some important documents detailing the inner workings of the hypergates for the inhabitants of Taiomi."))
   misn.finish(true)
end
