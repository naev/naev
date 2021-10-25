--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Divert the Dvaered Forces">
  <avail>
   <priority>3</priority>
   <chance>550</chance>
   <done>Diversion from Raelid</done>
   <location>Computer</location>
   <faction>FLF</faction>
   <faction>Frontier</faction>
   <cond>not diff.isApplied( "flf_dead" )</cond>
  </avail>
 </mission>
 --]]
--[[

   FLF diversion mission.

--]]
local fleet = require "fleet"
local fmt = require "format"
local flf = require "missions.flf.flf_common"

-- localization stuff

success_text = {}
success_text[1] = _("You receive a transmission from an FLF officer saying that the operation has completed, and you can now return to the base.")

pay_text = {}
pay_text[1] = _("The FLF officer in charge of the primary operation thanks you for your contribution and hands you your pay.")
pay_text[2] = _("You greet the FLF officer in charge of the primary operation, who seems happy that the mission was a success. You congratulate each other, and the officer hands you your pay.")

osd_desc    = {}
osd_desc[1] = _("Fly to the {sys} system")
osd_desc[2] = _("Engage and destroy Dvaered ships to get their attention")
osd_desc[3] = _("Return to FLF base")
osd_desc["__save"] = true


function create ()
   missys = flf.getTargetSystem()
   if not misn.claim( missys ) then misn.finish( false ) end

   local num_dvaereds = missys:presences()["Dvaered"]
   local num_empire = missys:presences()["Empire"]
   local num_flf = missys:presences()["FLF"]
   if num_dvaereds == nil then num_dvaereds = 0 end
   if num_empire == nil then num_empire = 0 end
   if num_flf == nil then num_flf = 0 end
   dv_attention_target = num_dvaereds / 50
   credits = 200 * (num_dvaereds + num_empire - num_flf) * system.cur():jumpDist( missys, true ) / 3
   credits = credits + rnd.sigma() * 10e3
   reputation = math.max( (num_dvaereds + num_empire - num_flf) / 25, 1 )
   if credits < 10e3 then misn.finish( false ) end

   -- Set mission details
   misn.setTitle( _("FLF: Diversion in %s"):format( missys:name() ) )
   misn.setDesc( _("A fleet of FLF ships will be conducting an operation against the Dvaered forces. Create a diversion from this operation by wreaking havoc in the nearby %s system."):format( missys:name() ) )
   misn.setReward( fmt.credits( credits ) )
   marker = misn.markerAdd( missys, "computer" )
end


function accept ()
   misn.accept()

   osd_desc[1] = fmt.f( osd_desc[1], {sys=missys} )
   misn.osdCreate( _("FLF Diversion"), osd_desc )

   dv_attention = 0
   dv_coming = false
   job_done = false

   hook.enter( "enter" )
   hook.jumpout( "leave" )
   hook.land( "leave" )
end


function enter ()
   if not job_done then
      if system.cur() == missys then
         misn.osdActive( 2 )
         update_dv()
      else
         misn.osdActive( 1 )
         dv_attention = 0
      end
   end
end


function leave ()
   if update_dv_hook ~= nil then hook.rm( update_dv_hook ) end
end


function update_dv ()
   for i, j in ipairs( pilot.get( { faction.get("Dvaered") } ) ) do
      hook.pilot( j, "attacked", "pilot_attacked_dv" )
      hook.pilot( j, "death", "pilot_death_dv" )
   end
   update_dv_hook = hook.timer( 3.0, "update_dv" )
end


function add_attention( p )
   p:setHilight( true )

   if not job_done then
      dv_attention = dv_attention + 1
      if dv_attention >= dv_attention_target and dv_attention - 1 < dv_attention_target then
         if success_hook ~= nil then hook.rm( success_hook ) end
         success_hook = hook.timer( 30.0, "timer_mission_success" )
      end

      hook.pilot( p, "jump", "rm_attention" )
      hook.pilot( p, "land", "rm_attention" )
   end
end


function rm_attention ()
   dv_attention = math.max( dv_attention - 1, 0 )
   if dv_attention < dv_attention_target then
      if success_hook ~= nil then hook.rm( success_hook ) end
   end
end


function pilot_attacked_dv( _p, attacker )
   if (attacker == player.pilot() or attacker:leader() == player.pilot())
         and not dv_coming and rnd.rnd() < 0.1 then
      dv_coming = true
      hook.timer( 10.0, "timer_spawn_dv" )
   end
end


function pilot_death_dv( _p, attacker )
   if (attacker == player.pilot() or attacker:leader() == player.pilot())
         and not dv_coming then
      dv_coming = true
      hook.timer( 10.0, "timer_spawn_dv" )
   end
end


function timer_spawn_dv ()
   dv_coming = false
   if not job_done then
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

      player.msg( _("%s has warped in!"):format( _(fleetname) ) )
      for i, j in ipairs( fleet.add( 1,  ships, "Dvaered" ) ) do
         add_attention( j )
      end
   end
end


function timer_mission_success ()
   if dv_attention >= dv_attention_target then
      job_done = true
      misn.osdActive( 3 )
      if marker ~= nil then misn.markerRm( marker ) end
      if update_dv_hook ~= nil then hook.rm( update_dv_hook ) end
      hook.land( "land" )
      tk.msg( "", success_text[ rnd.rnd( 1, #success_text ) ] )
   end
end


function land ()
   if planet.cur():faction() == faction.get("FLF") then
      tk.msg( "", pay_text[ rnd.rnd( 1, #pay_text ) ] )
      player.pay( credits )
      faction.get("FLF"):modPlayer( reputation )
      misn.finish( true )
   end
end
