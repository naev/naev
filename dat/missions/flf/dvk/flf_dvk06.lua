--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Assault on Haleb">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>2</priority>
  <chance>30</chance>
  <done>Alliance of Inconvenience</done>
  <location>Bar</location>
  <faction>FLF</faction>
  <cond>faction.playerStanding("FLF") &gt;= 80</cond>
 </avail>
 <notes>
  <campaign>Save the Frontier</campaign>
 </notes>
</mission>
--]]
--[[

   Assault on Haleb

--]]
local fmt = require "format"
local fleet = require "fleet"
local flf = require "missions.flf.flf_common"

-- Localization stuff

osd_desc    = {}
osd_desc[1] = _("Fly to the %s system and meet with the group of FLF ships")
osd_desc[2] = _("Wait until the coast is clear, then hail one of your wingmates")
osd_desc[3] = _("Attack Raglan Outpost until it is destroyed")
osd_desc[4] = _("Return to FLF base")
osd_desc["__save"] = true

function create ()
   missys = system.get( "Haleb" )
   if not misn.claim( missys ) then
      misn.finish( false )
   end

   misn.setNPC( _("Benito"), "flf/unique/benito.webp", _("Benito is seated at a table with several other FLF soldiers. She motions for you to come over.") )
end


function accept ()
   if tk.yesno( _("Here we go again"), _([["%s, we were just saying you should join in on this one! It's another great victory against the Dvaered oppressors, and we'd like you to lead the way to victory once again! Are you in?"]]):format( player.name() ) ) then
      tk.msg( _("Another Decisive Strike"), _([["Excellent!" You take a seat. "So once again, our mission today is the destruction of a loathed Dvaered base: Raglan Outpost! The plan is pretty much the same as before: we have tasked a group of pirates with creating a disturbance nearby, and we have planted a bomb within the outpost to aid in its destruction. You just need to decide when to strike and let your teammates know.
    The one thing that will be different, though, is that you're likely to find more Dvaered ships guarding Raglan Outpost compared to Raelid Outpost, and it might be a little harder to destroy. So be extra careful!" Time to get your ship ready for battle, then.]]) )

      misn.accept()

      osd_desc[1] = osd_desc[1]:format( missys:name() )
      misn.osdCreate( _("Assault on Haleb"), osd_desc )
      misn.setTitle( _("Assault on Haleb") )
      misn.setDesc( _("Join with the other FLF pilots for the assault on Raglan Outpost."):format( missys:name() ) )
      marker = misn.markerAdd( missys, "plot" )
      misn.setReward( _("Another great victory against the Dvaereds") )

      credits = 750e3
      reputation = 5

      started = false
      attacked_station = false
      completed = false

      hook.enter( "enter" )
   else
      tk.msg( _("Not This Time"), _([["Okay. Just let me know if you change your mind."]]) )
      misn.finish( false )
   end
end


function enter ()
   if not completed then
      started = false
      attacked_station = false
      misn.osdActive( 1 )
      if timer_start_hook ~= nil then hook.rm( timer_start_hook ) end
      if timer_pirates_hook ~= nil then hook.rm( timer_pirates_hook ) end

      if diff.isApplied( "raglan_outpost_death" ) then
         diff.remove( "raglan_outpost_death" )
      end

      if system.cur() == missys then
         pilot.clear()
         pilot.toggleSpawn( false )

         local ro = planet.get( "Raglan Outpost" )

         -- Spawn Raglan Outpost ship
         dv_base = pilot.add( "Raglan Outpost", "Dvaered", ro:pos() , nil, {ai="dvaered_norun"} )
         dv_base:outfitRm( "all" )
         dv_base:outfitRm( "cores" )
         dv_base:outfitAdd( "Dummy Systems" )
         dv_base:outfitAdd( "Dummy Plating" )
         dv_base:outfitAdd( "Dummy Engine" )
         dv_base:control()
         dv_base:setNoDisable()
         dv_base:setNoboard()
         dv_base:setNoLand()
         dv_base:setVisible()
         dv_base:setHilight()
         hook.pilot( dv_base, "attacked", "pilot_attacked_station" )
         hook.pilot( dv_base, "death", "pilot_death_station" )

         -- Spawn Dvaered ships
         local dv_ships = {
            "Dvaered Goddard", "Dvaered Ancestor", "Dvaered Phalanx", "Dvaered Vigilance", "Dvaered Ancestor", "Dvaered Phalanx", "Dvaered Vigilance", "Dvaered Ancestor", "Dvaered Ancestor",
            "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta",
            "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Vendetta"}
         dv_fleet = fleet.add( 1, dv_ships, "Dvaered", ro:pos(), nil, {ai="dvaered_norun"} )

         for i, j in ipairs( dv_fleet ) do
            j:control()
            j:setVisible()
            hook.pilot( j, "attacked", "pilot_attacked" )
         end

         -- Spawn FLF ships
         local jmp, jmp2
         jmp, jmp2 = jump.get( "Haleb", "Theras" )
         flf_fleet = fleet.add( 4, {"Vendetta", "Vendetta", "Lancelot"}, "FLF", jmp:pos() )

         for i, j in ipairs( flf_fleet ) do
            j:control()
            j:brake()
            j:face( dv_base:pos(), true )
            j:setVisplayer()
            j:setHilight()
            hook.pilot( j, "attacked", "pilot_attacked" )
         end

         timer_start_hook = hook.timer( 4.0, "timer_start" )
         diff.apply( "raglan_outpost_death" )
      end
   end
end


function timer_start ()
   if timer_start_hook ~= nil then hook.rm( timer_start_hook ) end

   local player_pos = player.pos()
   local proximity = false
   for i, j in ipairs( flf_fleet ) do
      local dist = player_pos:dist( j:pos() )
      if dist < 500 then proximity = true end
   end

   if proximity then
      started = true
      flf_fleet[1]:comm( _("You're just in time, %s! The chaos is just about to unfold."):format( player.name() ) )
      timer_pirates_hook = hook.timer( 4.0, "timer_pirates" )
      misn.osdActive( 2 )

      for i, j in ipairs( flf_fleet ) do
         j:setHilight( false )
         hook.hail( "hail" )
      end

      local ships = { "Llama", "Gawain", "Llama", "Koala", "Mule" }
      local factions = { "Independent", "Independent", "Trader", "Trader", "Trader" }
      local pilotnames = { _("Llama"), _("Gawain"), _("Trader Llama"), _("Trader Koala"), _("Trader Mule") }
      local src = system.get( "Triap" )
      civ_fleet = fleet.add( 16, ships, factions, src, pilotnames )

      local dest = system.get( "Slaccid" )
      for i, j in ipairs( civ_fleet ) do
         j:control()
         j:hyperspace( dest )
         j:setVisible()
         hook.pilot( j, "attacked", "pilot_attacked_civilian" )
      end
   else
      timer_start_hook = hook.timer( 0.05, "timer_start" )
   end
end


function timer_pirates ()
   civ_fleet[1]:comm( _("Help! SOS! We are under attack! In need of immediate assistance!") )

   local src = system.get( "Triap" )
   local ships = {
      "Hyena", "Pirate Shark", "Pirate Admonisher",
      "Pirate Vendetta", "Pirate Ancestor" }
   local pilotnames = {
      _("Pirate Hyena"), _("Pirate Shark"), _("Pirate Admonisher"),
      _("Pirate Vendetta"), _("Pirate Ancestor") }

   pir_boss = pilot.add( "Pirate Kestrel", "Dreamer Clan", src )
   pir_fleet = fleet.add( 9, ships, "Dreamer Clan", src, pilotnames )
   pir_fleet[ #pir_fleet + 1 ] = pir_boss
   hook.pilot( pir_boss, "death", "pilot_death_kestrel" )

   for i, j in ipairs( pir_fleet ) do
      j:control()
      j:setVisible()
      j:setFriendly()
      j:attack()
      hook.pilot( j, "attacked", "pilot_attacked" )
   end

   for i, j in ipairs( dv_fleet ) do
      if j:exists() then
         j:attack( pir_boss )
      end
   end
end


function hail ()
   player.commClose()
   if not attacked_station then
      local comm_done = false
      for i, j in ipairs( flf_fleet ) do
         if j:exists() then
            j:attack( dv_base )
            if not comm_done then
               j:comm( _("You heard the boss! Let's grind that station to dust!") )
               comm_done = true
            end
         end
      end
      attacked_station = true
      misn.osdActive( 3 )
   end
end


function pilot_attacked( pilot, _attacker, _arg )
   pilot:control( false )
end


function pilot_attacked_civilian( pilot, attacker, _arg )
   pilot:control( false )
   attacker:control( false )
end


function pilot_attacked_station( _pilot, _attacker, _arg )
   for i, j in ipairs( dv_fleet ) do
      if j:exists() then
         j:control( false )
         j:setHostile()
      end
   end
   for i, j in ipairs( flf_fleet ) do
      if j:exists() then
         j:setVisible()
      end
   end
end


function pilot_death_civilian( _pilot, _attacker, _arg )
   for i, j in ipairs( pir_fleet ) do
      if j:exists() then
         j:control( false )
      end
   end
end


function pilot_death_kestrel( _pilot, _attacker, _arg )
   for i, j in ipairs( dv_fleet ) do
      if j:exists() then
         j:control( false )
      end
   end
end


function pilot_death_station( pilot, _attacker, _arg )
   for i, j in ipairs( flf_fleet ) do
      if j:exists() then
         j:control( false )
         j:changeAI( "flf" )
      end
   end
   for i, j in ipairs( pir_fleet ) do
      if j:exists() then
         j:control( false )
      end
   end

   completed = true
   pilot.toggleSpawn( true )
   misn.osdActive( 4 )
   if marker ~= nil then misn.markerRm( marker ) end
   hook.land( "land" )
end


function land ()
   if planet.cur():faction() == faction.get("FLF") then
      tk.msg( _("Another Day, Another Victory"), _([[If your comrades were happy about your victory at Raelid, they are ecstatic about your victory at Haleb. As you enter the station, you are met with cheers from what seems to be everyone. It takes you longer than usual to make it to Benito as a result. "Congratulations," she says. "That was an astounding victory, sure to set back the Dvaered oppressors substantially! This is the first time we've pushed them out of Frontier space, and for that, we all thank you. If you haven't noticed, you've made yourself into a bit of a hero!
    "Here is your pay, %s. May we have another great victory later on! Down with the oppressors!" You exchange some more words with Benito, party with the others for a period or two, and then make your way back to your ship for some much-needed rest.]]):format( player.name() ) )
      finish()
   end
end


function finish ()
   player.pay( credits )
   flf.setReputation( 90 )
   faction.get("FLF"):modPlayer( reputation )
   flf.addLog( _([[You led the charge to destroy Raglan Outpost, a source of deep penetration of Dvaered forces into the Frontier. As a result, Dvaered forces have started to be pushed out of Frontier space, the first time the FLF has ever done so and a major victory for the Frontier.]]) )
   misn.finish( true )
end


function abort ()
   if completed then
      finish()
   else
      if diff.isApplied( "raglan_outpost_death" ) then
         diff.remove( "raglan_outpost_death" )
      end
      if dv_base ~= nil and dv_base:exists() then
         dv_base:rm()
      end
      misn.finish( false )
   end
end

