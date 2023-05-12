--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Divert the Dvaered Forces">
 <priority>3</priority>
 <chance>550</chance>
 <done>Diversion from Raelid</done>
 <location>Computer</location>
 <faction>FLF</faction>
 <faction>Frontier</faction>
 <cond>not diff.isApplied( "flf_dead" )</cond>
</mission>
 --]]
--[[

   FLF diversion mission.

--]]
local fleet = require "fleet"
local fmt = require "format"
local flf = require "missions.flf.flf_common"

-- luacheck: globals success_text pay_text (inherited by missions that require this one, TODO get rid of this hack)

success_text = {
   _("You receive a transmission from an FLF officer saying that the operation was completed, and you can now return to the base."),
}

pay_text = {
   _("The FLF officer in charge of the primary operation thanks you for your contribution and hands you your pay."),
   _("You greet the FLF officer in charge of the primary operation, who seems happy that the mission was a success. You congratulate each other and the officer hands you your pay."),
}

mem.osd_desc    = {
   _("Fly to the {sys} system"),
   _("Engage and destroy Dvaered ships to get their attention"),
   _("Return to FLF base"),
}


function create ()
   mem.missys = flf.getTargetSystem()
   if not misn.claim( mem.missys ) then misn.finish( false ) end

   local num_dvaereds = mem.missys:presences()["Dvaered"]
   local num_empire = mem.missys:presences()["Empire"]
   local num_flf = mem.missys:presences()["FLF"]
   if num_dvaereds == nil then num_dvaereds = 0 end
   if num_empire == nil then num_empire = 0 end
   if num_flf == nil then num_flf = 0 end
   mem.dv_attention_target = num_dvaereds / 50
   mem.credits = 200 * (num_dvaereds + num_empire - num_flf) * system.cur():jumpDist( mem.missys, true ) / 3
   mem.credits = mem.credits + rnd.sigma() * 10e3
   mem.reputation = math.max( (num_dvaereds + num_empire - num_flf) / 25, 1 )
   if mem.credits < 10e3 then misn.finish( false ) end

   -- Set mission details
   misn.setTitle( fmt.f( _("FLF: Diversion in {sys}"), {sys=mem.missys} ) )
   misn.setDesc( fmt.f( _("A fleet of FLF ships will be conducting an operation against the Dvaered forces. Create a diversion from this operation by wreaking havoc in the nearby {sys} system."), {sys=mem.missys} ) )
   misn.setReward( mem.credits )
   mem.marker = misn.markerAdd( mem.missys, "computer" )
end


function accept ()
   misn.accept()

   mem.osd_desc[1] = fmt.f( mem.osd_desc[1], {sys=mem.missys} )
   misn.osdCreate( _("FLF Diversion"), mem.osd_desc )

   mem.dv_attention = 0
   mem.dv_coming = false
   mem.job_done = false

   hook.enter( "enter" )
   hook.jumpout( "leave" )
   hook.land( "leave" )
end


function enter ()
   if not mem.job_done then
      if system.cur() == mem.missys then
         misn.osdActive( 2 )
         update_dv()
      else
         misn.osdActive( 1 )
         mem.dv_attention = 0
      end
   end
end


function leave ()
   hook.rm( mem.update_dv_hook )
end


function update_dv ()
   for i, j in ipairs( pilot.get( { faction.get("Dvaered") } ) ) do
      hook.pilot( j, "attacked", "pilot_attacked_dv" )
      hook.pilot( j, "death", "pilot_death_dv" )
   end
   mem.update_dv_hook = hook.timer( 3.0, "update_dv" )
end


local function add_attention( p )
   p:setHilight( true )

   if not mem.job_done then
      mem.dv_attention = mem.dv_attention + 1
      if mem.dv_attention >= mem.dv_attention_target and mem.dv_attention - 1 < mem.dv_attention_target then
         hook.rm( mem.success_hook )
         mem.success_hook = hook.timer( 30.0, "timer_mission_success" )
      end

      hook.pilot( p, "jump", "rm_attention" )
      hook.pilot( p, "land", "rm_attention" )
   end
end


function rm_attention ()
   mem.dv_attention = math.max( mem.dv_attention - 1, 0 )
   if mem.dv_attention < mem.dv_attention_target then
      hook.rm( mem.success_hook )
   end
end


function pilot_attacked_dv( _p, attacker )
   if attacker and attacker:withPlayer() and not mem.dv_coming and rnd.rnd() < 0.1 then
      mem.dv_coming = true
      hook.timer( 10.0, "timer_spawn_dv" )
   end
end


function pilot_death_dv( _p, attacker )
   if attacker and attacker:withPlayer() and not mem.dv_coming then
      mem.dv_coming = true
      hook.timer( 10.0, "timer_spawn_dv" )
   end
end


function timer_spawn_dv ()
   mem.dv_coming = false
   if not mem.job_done then
      local fleets = { {"Dvaered Vendetta"}, {"Dvaered Ancestor"}, {"Dvaered Phalanx"}, {"Dvaered Vigilance"}, {"Dvaered Goddard"},
                       {"Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Ancestor", "Dvaered Ancestor"},
                       {"Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Ancestor", "Dvaered Phalanx", "Dvaered Vigilance"} }
      local ships = fleets[ rnd.rnd( 1, #fleets ) ]
      local fleetname

      if #ships == 1 then
         fleetname = _(ships[1])
      elseif #ships < 5 then
         fleetname = _("Dvaered Small Patrol")
      else
         fleetname = _("Dvaered Big Patrol")
      end

      player.msg( fmt.f( _("{fleet} has warped in!"), {fleet=_(fleetname)} ) )
      for i, j in ipairs( fleet.add( 1,  ships, "Dvaered" ) ) do
         add_attention( j )
      end
   end
end


function timer_mission_success ()
   if mem.dv_attention >= mem.dv_attention_target then
      mem.job_done = true
      misn.osdActive( 3 )
      if mem.marker ~= nil then misn.markerRm( mem.marker ) end
      hook.rm( mem.update_dv_hook )
      hook.land( "land" )
      tk.msg( "", success_text[ rnd.rnd( 1, #success_text ) ] )
   end
end


function land ()
   if spob.cur():faction() == faction.get("FLF") then
      tk.msg( "", pay_text[ rnd.rnd( 1, #pay_text ) ] )
      player.pay( mem.credits )
      faction.get("FLF"):modPlayer( mem.reputation )
      misn.finish( true )
   end
end
