--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Waste Dump">
 <priority>9</priority>
 <chance>10</chance>
 <location>Computer</location>
 <cond>
   local f = spob.cur():faction()
   if f then
      local ft = f:tags()
      if ft.generic or ft.misn_cargo then
         return true
      end
   end
   return false
 </cond>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[
   Waste Dump
--]]
local fmt = require "format"
local pir = require "common.pirate"
local vntk = require "vntk"
local lmisn = require "lmisn"

local text = {
   _("The waste containers are loaded onto your ship and you are paid {credits}. You begin to wonder if accepting this job was really a good idea."),
   _("Workers pack your cargo hold full of as much garbage as it can carry, then hastily hand you a credit chip containing {credits}. Smelling the garbage, you immediately regret taking the job."),
   _("Your hold is crammed full with garbage and you are summarily paid {credits}. By the time the overpowering stench emanating from your cargo hold reaches you, it's too late to back down; you're stuck with this garbage until you can find some place to get rid of it."),
}

local finish_text = {
   _("You drop the garbage off, relieved to have it out of your ship."),
   _("You finally drop off the garbage and proceed to disinfect yourself and your cargo hold to the best of your ability."),
   _("Finally, the garbage leaves your ship and you breathe a sigh of relief."),
   _("Wrinkling your nose in disgust, you finally rid yourself of the waste containers you have been charged with disposing of."),
}

local abort_text = {
   _("Sick and tired of smelling garbage, you illegally jettison the waste containers into space, hoping that no one notices."),
   _("You decide that the nearest waste dump location is too far away for you to bother to go to and simply jettison the containers of waste. You hope you don't get caught."),
   _("You dump the waste containers into space illegally, noting that you should make sure not to get caught by authorities."),
}

-- List of possible waste dump planets.
local dest_planets = {}
for k,v in ipairs(spob.getAll()) do
   local t = v:tags()
   if t.garbage then
      table.insert( dest_planets, v )
   end
end
table.sort( dest_planets, function ( a, b )
   return a:system():jumpDist() < b:system():jumpDist()
end )

function create ()
   local scur = system.cur()
   local dist = math.huge
   for i,p in ipairs(dest_planets) do
      dist = math.min( dist, scur:jumpDist(p:system()) )
   end
   -- failed to create
   if dist == math.huge then
      misn.finish(false)
   end

   -- Note: this mission makes no system claims

   mem.credits_factor = 500 + 300 * dist
   mem.credits_mod = 10e3 * rnd.sigma()

   for i,p in ipairs( dest_planets ) do
      misn.markerAdd( p, "computer" )
   end

   -- Set mission details
   misn.setTitle( _("Waste Dump") )
   misn.setDesc(_("Take as many waste containers as your ship can hold and drop them off at any authorized garbage collection facility. You will be paid immediately, but any attempt to illegally get rid of the waste will be severely punished if you are caught."))
   misn.setReward( fmt.f(_("{credits} per tonne"), {credits=fmt.credits(mem.credits_factor)} ) )
end

function accept ()
   misn.accept()

   local q = player.pilot():cargoFree()
   mem.credits = mem.credits_factor * q + mem.credits_mod

   lmisn.sfxMoney()
   local txt = text[ rnd.rnd( 1, #text ) ]
   vntk.msg(_("Waste Containers Loaded"), fmt.f( txt, {credits = fmt.credits( mem.credits ) } ) )

   local c = commodity.new( N_("Waste Containers"), N_("A bunch of waste containers leaking all sorts of indescribable liquids. You hope they don't leak onto your ship.") )
   mem.cid = misn.cargoAdd( c, q )
   player.pay( mem.credits )

   misn.osdCreate( _("Waste Dump"), {_("Land on any garbage collection facility (indicated on your map) to drop off the Waste Containers")} )

   hook.land( "land" )
end

function land ()
   for i,p in ipairs(dest_planets) do
      if p == spob.cur() then
         local txt = finish_text[ rnd.rnd( 1, #finish_text ) ]
         lmisn.sfxVictory()
         vntk.msg(_("No More Garbage"), txt)
         pir.reputationNormalMission(rnd.rnd(2,3))
         misn.finish( true )
      end
   end
end

function abort ()
   if player.isLanded() then
      local fine = math.min( player.credits(), 2 * mem.credits )
      local msg
      local spb = spob.get()
      if spb:services().inhabited then
         msg = fmt.f(_("In your desperation to rid yourself of the garbage, you clumsily eject it from your cargo pod while you are still landed. Garbage spills all over the hangar and local officials immediately take notice. After you apologize profusely and explain the situation was an accident, the officials let you off with a fine of {credits}."), {credits=fmt.credits(fine)} )
      else
         msg = fmt.f(_("Thinking {spob} to be devoid of people, you eject your cargo pod while landed. To your surprise, you find a wandering environmentalist knocking on your ship airlock. You make the mistake of opening the airlock and letting them in. After having to hear a tirade about how you are polluting pristine locations around the galaxy, you end up paying them {credits} to leave and clean up the mess you made."), {credits=fmt.credits(fine), spob=spb} )
      end
      vntk.msg(_("Dirty Deed"), msg)
      player.pay( -fine )
      misn.finish( false )

   else
      local txt = abort_text[ rnd.rnd( 1, #abort_text ) ]
      vntk.msg(_("Dirty Deed"), txt)

      misn.cargoJet( mem.cid )

      -- Make everyone angry
      for i,p in ipairs(pilot.get()) do
         if not p:withPlayer() then
            p:setHostile()
         end
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
         choices = { "Za'lek Mephisto", "Za'lek Diablo", "Za'lek Demon" }
      elseif f == faction.get( "Sirius" ) then
         choices = { "Sirius Preacher", "Sirius Divinity", "Sirius Dogma" }
      elseif f == faction.get( "Frontier" ) then
         choices = { "Phalanx", "Lancelot", "Ancestor" }
      elseif f == faction.get( "Thurion" ) then
         choices = { "Thurion Apprehension", "Thurion Certitude" }
      elseif f == faction.get( "Proteron" ) then
         choices = { "Proteron Gauss", "Proteron Watson", "Proteron Archimedes" }
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

      local sjumps = system.cur():jumps()
      for n = 1,rnd.rnd( 2, 4 ) do
         for i,j in ipairs(sjumps) do
            local p = pilot.add( choices[ rnd.rnd( 1, #choices ) ], f, j:dest() )
            p:setHostile()
         end
      end

      -- No landing, filthy waste dumper!
      player.landAllow( false, _("Get lost, waste dumping scum! We don't want you here!") )

      misn.finish( true )
   end
end
