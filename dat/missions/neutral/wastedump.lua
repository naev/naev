--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Waste Dump">
 <avail>
  <priority>9</priority>
  <done>Garbage Person</done>
  <chance>100</chance>
  <location>Computer</location>
  <faction>Dvaered</faction>
  <faction>Empire</faction>
  <faction>Frontier</faction>
  <faction>Goddard</faction>
  <faction>Independent</faction>
  <faction>Sirius</faction>
  <faction>Soromid</faction>
  <faction>Thurion</faction>
  <faction>Traders Guild</faction>
  <faction>Za'lek</faction>
 </avail>
</mission>
--]]
--[[

   Waste Dump

--]]
local fmt = require "format"
local pir = require "common.pirate"
local vntk = require "vntk"

-- luacheck: globals land takeoff (Hook functions passed by name)

local text = {}
text[1] = _("The waste containers are loaded onto your ship and you are paid {credits}. You begin to wonder if accepting this job was really a good idea.")
text[2] = _("Workers pack your cargo hold full of as much garbage as it can carry, then hastily hand you a credit chip containing {credits}. Smelling the garbage, you immediately regret taking the job.")
text[3] = _("Your hold is crammed full with garbage and you are summarily paid {credits}. By the time the overpowering stench emanating from your cargo hold reaches you, it's too late to back down; you're stuck with this garbage until you can find some place to get rid of it.")

local finish_text = {}
finish_text[1] = _("You drop the garbage off, relieved to have it out of your ship.")
finish_text[2] = _("You finally drop off the garbage and proceed to disinfect yourself and your cargo hold to the best of your ability.")
finish_text[3] = _("Finally, the garbage leaves your ship and you breathe a sigh of relief.")
finish_text[4] = _("Wrinkling your nose in disgust, you finally rid yourself of the waste containers you have been charged with disposing of.")

local abort_text = {}
abort_text[1] = _("Sick and tired of smelling garbage, you illegally jettison the waste containers into space, hoping that no one notices.")
abort_text[2] = _("You decide that the nearest waste dump location is too far away for you to bother to go to and simply jettison the containers of waste. You hope you don't get caught.")
abort_text[3] = _("You dump the waste containers into space illegally, noting that you should make sure not to get caught by authorities.")

-- List of possible waste dump planets.
local dest_planets = { "The Stinker", "Eiroik" }

function create ()
   local dist = nil
   for i, j in ipairs( dest_planets ) do
      local _p, sys = planet.getS( j )
      if dist == nil or system.cur():jumpDist(sys) < dist then
         dist = system.cur():jumpDist(sys)
      end
   end

   -- Note: this mission makes no system claims

   mem.credits_factor = 1e3 * dist
   mem.credits_mod = 10e3 * rnd.sigma()

   mem.landed = true

   for i, j in ipairs( dest_planets ) do
      local p = planet.get( j )
      misn.markerAdd( p, "computer" )
   end

   -- Set mission details
   misn.setTitle( _("Waste Dump") )
   misn.setDesc( _("Take as many waste containers as your ship can hold and drop them off at any authorised garbage collection facility. You will be paid immediately, but any attempt to illegally jettison the waste into space will be severely punished if you are caught.") )
   misn.setReward( fmt.f(_("{credits} per tonne"), {credits=fmt.credits(mem.credits_factor)} ) )
end


function accept ()
   misn.accept()

   local q = player.pilot():cargoFree()
   mem.credits = mem.credits_factor * q + mem.credits_mod

   local txt = text[ rnd.rnd( 1, #text ) ]
   vntk.msg( "", fmt.f( txt, {credits = fmt.credits( mem.credits ) } ) )

   local c = commodity.new( N_("Waste Containers"), N_("A bunch of waste containers leaking all sorts of indescribable liquids.") )
   mem.cid = misn.cargoAdd( c, q )
   player.pay( mem.credits )

   misn.osdCreate( _("Waste Dump"), {_("Land on any garbage collection facility (indicated on your map) to drop off the Waste Containers")} )

   hook.takeoff( "takeoff" )
   hook.land( "land" )
end


function takeoff ()
   mem.landed = false
end


function land ()
   mem.landed = true

   for i, j in ipairs( dest_planets ) do
      if planet.get(j) == planet.cur() then
         local txt = finish_text[ rnd.rnd( 1, #finish_text ) ]
         vntk.msg( "", txt )
         pir.reputationNormalMission(rnd.rnd(2,3))
         misn.finish( true )
      end
   end
end


function abort ()
   if mem.landed then
      misn.cargoRm( mem.cid )
      local fine = 2 * mem.credits
      vntk.msg( "", fmt.f(_("In your desperation to rid yourself of the garbage, you clumsily eject it from your cargo pod while you are still landed. Garbage spills all over the hangar and local officials immediately take notice. After you apologise profusely and explain the situation was an accident, the officials let you off with a fine of {credits}."), {credits=fmt.credits(fine)} ) )
      player.pay( -fine )
      misn.finish( false )
   else
      local txt = abort_text[ rnd.rnd( 1, #abort_text ) ]
      vntk.msg( "", txt )

      misn.cargoJet( mem.cid )

      -- Make everyone angry
      for i, j in ipairs( pilot.get() ) do
         j:setHostile()
      end

      -- Add some police!
      local presences = system.cur():presences()
      local f = nil
      if presences then
         local strongest_amount = 0
         for k, v in pairs( presences ) do
            if v > strongest_amount then
               f = faction.get(k)
               strongest_amount = v
            end
         end
      end

      local choices
      if f == faction.get( "Empire" ) then
         choices = { "Empire Lancelot", "Empire Admonisher", "Empire Pacifier" }
      elseif f == faction.get( "Goddard" ) then
         choices = { "Goddard", "Lancelot" }
      elseif f == faction.get( "Dvaered" ) then
         choices = { "Dvaered Vendetta", "Dvaered Phalanx", "Dvaered Vigilance" }
      elseif f == faction.get( "Soromid" ) then
         choices = { "Soromid Arx", "Soromid Vox", "Soromid Nyx", "Soromid Odium" }
      elseif f == faction.get( "Za'lek" ) then
         choices = { "Za'lek Hephaestus", "Za'lek Mephisto", "Za'lek Diablo", "Za'lek Demon" }
      elseif f == faction.get( "Sirius" ) then
         choices = { "Sirius Preacher", "Sirius Divinity", "Sirius Dogma" }
      elseif f == faction.get( "Frontier" ) then
         choices = { "Phalanx", "Lancelot", "Ancestor" }
      elseif f == faction.get( "Thurion" ) then
         choices = { "Thurion Apprehension", "Thurion Certitude" }
      elseif f == faction.get( "Proteron" ) then
         choices = { "Proteron Kahan", "Proteron Watson", "Proteron Archimedes" }
      elseif f == faction.get( "Collective" ) then
         choices = { "Collective Drone", "Collective Heavy Drone" }
      elseif f == faction.get( "FLF" ) then
         choices = { "Pacifier", "Vendetta", "Lancelot" }
      elseif pir.factionIsPirate(f) then
         choices = { "Pirate Kestrel", "Pirate Phalanx", "Pirate Admonisher" }
      else
         f = faction.get( "Mercenary" )
         choices = { "Lancelot", "Pacifier", "Ancestor", "Vendetta" }
      end

      for n = 1, rnd.rnd( 2, 4 ) do
         for i, j in ipairs( system.cur():jumps() ) do
            local p = pilot.add( choices[ rnd.rnd( 1, #choices ) ], f, j:dest() )
            p:setHostile()
         end
      end

      -- No landing, filthy waste dumper!
      player.allowLand( false, _("Get lost, waste dumping scum! We don't want you here!") )

      misn.finish( true )
   end
end
