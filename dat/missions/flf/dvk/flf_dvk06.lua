--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Assault on Haleb">
 <unique />
 <priority>2</priority>
 <chance>30</chance>
 <done>Alliance of Inconvenience</done>
 <location>Bar</location>
 <faction>FLF</faction>
 <cond>faction.playerStanding("FLF") &gt;= 80</cond>
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

local civ_fleet, dv_base, dv_fleet, flf_fleet, pir_boss, pir_fleet -- Non-persistent state
local finish -- Forward-declared functions

mem.osd_desc    = {}
mem.osd_desc[1] = _("Fly to the {sys} system and meet with the group of FLF ships")
mem.osd_desc[2] = _("Wait until the coast is clear, then hail one of your wingmates")
mem.osd_desc[3] = _("Attack Fort Raglan until it is destroyed")
mem.osd_desc[4] = _("Return to FLF base")

function create ()
   mem.missys = system.get( "Haleb" )
   if not misn.claim( mem.missys ) then
      misn.finish( false )
   end

   misn.setNPC( _("Benito"), "flf/unique/benito.webp", _("Benito is seated at a table with several other FLF soldiers. She motions for you to come over.") )
end


function accept ()
   if tk.yesno( _("Here we go again"), fmt.f( _([["{player}, we were just saying you should join in on this one! It's another great assault against the Dvaered oppressors, and we'd like you to lead the way to victory once again! Are you in?"]]), {player=player.name()} ) ) then
      tk.msg( _("Another Decisive Strike"), _([["Excellent!" You take a seat. "So once again, our mission today is the destruction of a loathed Dvaered base: Fort Raglan! The plan is pretty much the same as before: we have tasked a group of pirates with creating a disturbance nearby, and we have planted a bomb within the outpost to aid in its destruction. You just need to decide when to strike and let your teammates know.
    The one thing that will be different, though, is that you're likely to find more Dvaered ships guarding Fort Raglan compared to Fort Raelid, and it might be a little harder to destroy. So be extra careful!" Time to get your ship ready for battle, then.]]) )

      misn.accept()

      mem.osd_desc[1] = fmt.f( mem.osd_desc[1], {sys=mem.missys} )
      misn.osdCreate( _("Assault on Haleb"), mem.osd_desc )
      misn.setTitle( _("Assault on Haleb") )
      misn.setDesc( _("Join with the other FLF pilots for the assault on Fort Raglan.") )
      mem.marker = misn.markerAdd( mem.missys, "plot" )
      misn.setReward( _("Another great victory against the Dvaereds") )

      mem.credits = 750e3
      mem.reputation = 5

      mem.started = false
      mem.attacked_station = false
      mem.completed = false

      hook.enter( "enter" )
   else
      tk.msg( _("Not This Time"), _([["Okay. Just let me know if you change your mind."]]) )
      return
   end
end


function enter ()
   if not mem.completed then
      mem.started = false
      mem.attacked_station = false
      misn.osdActive( 1 )
      hook.rm( mem.timer_start_hook )
      hook.rm( mem.timer_pirates_hook )

      if diff.isApplied( "raglan_outpost_death" ) then
         diff.remove( "raglan_outpost_death" )
      end

      if system.cur() == mem.missys then
         pilot.clear()
         pilot.toggleSpawn( false )

         local ro = spob.get( "Fort Raglan" )

         -- Spawn Fort Raglan ship
         dv_base = pilot.add( "Fort Raglan", "Dvaered", ro:pos() , nil, {ai="dvaered_norun", naked=true} )
         dv_base:outfitRm( "all" )
         dv_base:outfitRm( "cores" )
         dv_base:outfitAdd( "Dummy Systems" )
         dv_base:outfitAdd( "Dummy Plating" )
         dv_base:outfitAdd( "Dummy Engine" )
         dv_base:control()
         dv_base:setNoDisable()
         dv_base:setNoBoard()
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
         local jmp = jump.get( "Haleb", "Theras" )
         flf_fleet = fleet.add( 4, {"Vendetta", "Vendetta", "Lancelot"}, "FLF", jmp:pos() )

         for i, j in ipairs( flf_fleet ) do
            j:control()
            j:brake()
            j:face( dv_base:pos(), true )
            j:setVisplayer()
            j:setHilight()
            hook.pilot( j, "attacked", "pilot_attacked" )
         end

         mem.timer_start_hook = hook.timer( 4.0, "timer_start" )
         diff.apply( "raglan_outpost_death" )
      end
   end
end


function timer_start ()
   hook.rm( mem.timer_start_hook )

   local player_pos = player.pos()
   local proximity = false
   for i, j in ipairs( flf_fleet ) do
      local dist = player_pos:dist( j:pos() )
      if dist < 500 then proximity = true end
   end

   if proximity then
      mem.started = true
      flf_fleet[1]:comm( fmt.f( _("You're just in time, {player}! The chaos is just about to unfold."), {player=player.name()} ) )
      mem.timer_pirates_hook = hook.timer( 4.0, "timer_pirates" )
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
         hook.pilot( j, "death", "pilot_death_civilian" )
      end
   else
      mem.timer_start_hook = hook.timer( 0.05, "timer_start" )
   end
end


function timer_pirates ()
   civ_fleet[1]:comm( _("Help! SOS! We are under attack! In need of immediate assistance!") )

   local src = system.get( "Triap" )
   local ships = {
      "Pirate Hyena", "Pirate Shark", "Pirate Admonisher",
      "Pirate Vendetta", "Pirate Ancestor" }

   pir_boss = pilot.add( "Pirate Kestrel", "Dreamer Clan", src )
   pir_fleet = fleet.add( 9, ships, "Dreamer Clan", src )
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
   if not mem.attacked_station then
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
      mem.attacked_station = true
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

   mem.completed = true
   pilot.toggleSpawn( true )
   misn.osdActive( 4 )
   if mem.marker ~= nil then misn.markerRm( mem.marker ) end
   hook.land( "land" )
end


function land ()
   if spob.cur():faction() == faction.get("FLF") then
      tk.msg( _("Another Day, Another Victory"), fmt.f( _([[If your comrades were happy about your success at Raelid, they are ecstatic about your victory at Haleb. As you enter the station, you are met with cheers from what seems to be everyone. As a result, it takes you longer than usual to make it to Benito. "Congratulations," she says. "That was an astounding victory, sure to set back the Dvaered oppressors substantially! This is the first time we've pushed them out of Frontier space, and for that, we all thank you. If you haven't noticed, you've made yourself into a bit of a hero!
    "Here is your pay, {player}. May we have another daring operation later on! Down with the oppressors!" You exchange some more words with Benito, party with the others for a period or two, and then make your way back to your ship for some much-needed rest.]]), {player=player.name()} ) )
      finish()
   end
end


function finish ()
   player.pay( mem.credits )
   flf.setReputation( 90 )
   faction.get("FLF"):modPlayer( mem.reputation )
   flf.addLog( _([[You led the charge to destroy Fort Raglan, a source of deep penetration of Dvaered forces into the Frontier. As a result, Dvaered forces have started to be pushed out of Frontier space, the first time the FLF has ever done so and a major victory for the Frontier.]]) )
   misn.finish( true )
end


function abort ()
   if mem.completed then
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
