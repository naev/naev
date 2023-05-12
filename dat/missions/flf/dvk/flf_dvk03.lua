--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Assault on Raelid">
 <unique />
 <priority>2</priority>
 <chance>30</chance>
 <done>FLF Pirate Alliance</done>
 <location>Bar</location>
 <faction>FLF</faction>
 <cond>faction.playerStanding("FLF") &gt;= 50</cond>
 <notes>
   <provides name="The Empire and the FLF are enemies">Because they're caught in the battle</provides>
   <campaign>Save the Frontier</campaign>
 </notes>
</mission>
 --]]
--[[

   Assault on Raelid

--]]
local fleet = require "fleet"
local fmt = require "format"
local flf = require "missions.flf.flf_common"

local civ_fleet, dv_base, dv_fleet, flf_fleet, pir_boss, pir_fleet -- Non-persistent state
local finish -- Forward-declared functions

mem.osd_desc    = {}
mem.osd_desc[2] = _("Wait until the coast is clear, then hail one of your wingmates")
mem.osd_desc[3] = _("Attack Fort Raelid until it is destroyed")
mem.osd_desc[4] = _("Return to FLF base")

function create ()
   mem.missys = system.get( "Raelid" )
   if not misn.claim( mem.missys ) then
      misn.finish( false )
   end

   misn.setNPC( _("Benito"), "flf/unique/benito.webp", _("Benito is seated at a table with several other FLF soldiers. She motions for you to come over.") )
end


function accept ()
   if tk.yesno( _("The Next Level"), fmt.f( _([["Hello there, {player}! You're just in time. We were just discussing our next operation against the Dvaered oppressors! Do you want in?"]]), {player=player.name()} ) ) then
      tk.msg( _("A Decisive Strike"), _([[You take a seat among the group. "Fantastic!" says Benito. "Let me get you caught up, then. Do you remember that mission in Raelid you helped with a while back?" You nod. You were wondering what you were actually creating a diversion from. "Yes, well, I never got around to telling you what we actually did there. See, we've been wanting to destroy Fort Raelid for some time, mostly because it's often used as a front for trying to scout us out. So while you were getting the attention of those Dvaereds, we rigged a special bomb and covertly installed it onto the outpost!"]]) )
      tk.msg( _("A Decisive Strike"), fmt.f( _([["Now, the bomb is not perfect. Given how hastily we had to install the thing, we could not make it so that it could be detonated remotely. Instead, it has to be detonated manually, by blasting the station repeatedly. Shooting it down, in other words.
    "So here's the plan. We have hired a large group of pirates to help us out by creating a massive disturbance far away from our target. You are to wait until the coast is clear, then swarm in and attack the outpost with all you've got. You, {player}, will lead the charge. You have to determine the optimal time, when the Dvaereds are far enough away for you to initiate the attack, but before the pirates are inevitably overwhelmed. Simply hail one of the others when it's time to attack, then make a beeline for Fort Raelid and shoot at it with all you've got!"]]), {player=player.name()} ) )
      tk.msg( _("A Decisive Strike"), _([["You guys are some of our best pilots, so try not to get killed, eh? A moment of triumph is upon us! Down with the oppressors!" The last line earns Benito a cheer from the crowd. Well, time to get your ship ready for the battle.]]) )

      misn.accept()

      mem.osd_desc[1] = fmt.f( _("Fly to the {sys} system and meet with the group of FLF ships"), {sys=mem.missys} )
      misn.osdCreate( _("Assault on Raelid"), mem.osd_desc )
      misn.setTitle( _("Assault on Raelid") )
      misn.setDesc( _("Join with the other FLF pilots for the assault on Fort Raelid.") )
      mem.marker = misn.markerAdd( mem.missys, "plot" )
      misn.setReward( _("A great victory against the Dvaereds") )

      mem.credits = 300e3
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

      if diff.isApplied( "raelid_outpost_death" ) then
         diff.remove( "raelid_outpost_death" )
      end

      if system.cur() == mem.missys then
         pilot.clear()
         pilot.toggleSpawn( false )

         local ro, ms, nf

         ro = spob.get( "Fort Raelid" )
         ms = spob.get( "Marius Enclave" )

         -- Spawn Fort Raelid ship
         dv_base = pilot.add( "Fort Raelid", "Dvaered", ro:pos() , nil, {ai="dvaered_norun", naked=true} )
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

         -- Spawn Dvaered and Empire ships
         dv_fleet = {}

         local empire_lge_attack = {"Empire Lancelot", "Empire Lancelot", "Empire Lancelot", "Empire Lancelot", "Empire Lancelot", "Empire Lancelot", "Empire Lancelot",
                                    "Empire Admonisher", "Empire Admonisher", "Empire Admonisher",
                                    "Empire Pacifier",
                                    "Empire Hawking",
                                    "Empire Peacemaker"}
         nf = fleet.add( 1, empire_lge_attack, "Empire", ms:pos(), nil, {ai="empire_norun"} )
         for i, j in ipairs( nf ) do
            dv_fleet[ #dv_fleet + 1 ] = j
         end

         local dv_big_patrol = { "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Ancestor", "Dvaered Phalanx", "Dvaered Vigilance" }
         nf = fleet.add( 1, dv_big_patrol, "Dvaered", ro:pos(), nil, {ai="dvaered_norun"} )
         for i, j in ipairs( nf ) do
            dv_fleet[ #dv_fleet + 1 ] = j
         end

         for i, j in ipairs( dv_fleet ) do
            j:control()
            j:setVisible()
            hook.pilot( j, "attacked", "pilot_attacked" )
         end

         -- Spawn FLF ships
         local jmp = jump.get( "Raelid", "Zacron" )
         flf_fleet = fleet.add( 14, "Vendetta", "FLF", jmp:pos() )

         for i, j in ipairs( flf_fleet ) do
            j:control()
            j:brake()
            j:face( dv_base:pos(), true )
            j:setVisplayer()
            j:setHilight()
            hook.pilot( j, "attacked", "pilot_attacked" )
         end

         mem.timer_start_hook = hook.timer( 4.0, "timer_start" )
         diff.apply( "raelid_outpost_death" )
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
      local src = system.get( "Zacron" )
      civ_fleet = fleet.add( 12, ships, factions, src, pilotnames )

      local dest = system.get( "Tau Prime" )
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

   local src = system.get( "Zacron" )
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


function pilot_death_station( _pilot, _attacker, _arg )
   hook.timer( 3.0, "timer_station" )
end


function timer_station ()
   tk.msg( _("Ominous Foretelling"), _([[As Fort Raelid erupts into a fantastic explosion before your very eyes, your comrades cheer. But then, you see something odd. Someone is hailing you... an Empire ship? Surely this can't be. Cautiously, you answer. The man whose face comes up on your view screen wastes no time.
    "So, you actually showed your face. I half expected you to run away and hide. But no matter." You try not to show any reaction to his icy stare. He continues.]]) )
   tk.msg( _("Ominous Foretelling"), _([["Terrorist, I'd bet you think this latest act of yours is a victory for you. Perhaps, for now, it is. But I assure you that the Empire will not ignore your activities any longer. I have already sent word to the Emperor, and he has authorized a declaration of your organization, the FLF, as an enemy of the Empire. Count the minutes on your fingers, terrorist. Your days are numbered."
    The Empire officer then immediately ceases communication, and you suddenly feel a chill down your spine. But one of your wingmates snaps you out of it. "Pay the Empire no mind," he says. "More importantly, we have to get out of here! We'll meet you at Sindbad."]]) )

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
   faction.get("Empire"):setPlayerStanding( -100 )
   diff.apply( "flf_vs_empire" )
   misn.osdActive( 4 )
   if mem.marker ~= nil then misn.markerRm( mem.marker ) end
   hook.land( "land" )
end


function land ()
   if spob.cur():faction() == faction.get("FLF") then
      tk.msg( _("Victory on the Horizon"), _([[As you return to the base, you are welcomed with all manner of cheers and enthusiasm. You can understand why, too; this is a huge victory for the FLF, and surely just one of many victories to come. But still...
    You manage to make your way over to Benito, who is clearly pleased with the outcome. "Outstanding job!" she says. "That base has been a burden on us for so long. Now it is gone, 100% gone! I don't think I need to tell you how fantastic a triumph this is. Victory is within our grasp!" That's when all doubt is erased from your mind. She's right; so what if the Empire is against you now? You exchange some more words with Benito, after which she hands you your pay for a job well done and excuses herself. You, on the other hand, stay behind to celebrate for a few more periods before finally excusing yourself.]]) )
      finish()
   end
end


function finish ()
   player.pay( mem.credits )
   flf.setReputation( 70 )
   faction.get("FLF"):modPlayer( mem.reputation )
   flf.addLog( _([[You led the effort to destroy the hated Dvaered base, Fort Raelid, a major victory for the FLF. This act led to the Empire listing you and the FLF as an enemy of the Empire.]]) )
   misn.finish( true )
end


function abort ()
   if mem.completed then
      finish()
   else
      if diff.isApplied( "raelid_outpost_death" ) then
         diff.remove( "raelid_outpost_death" )
      end
      if diff.isApplied( "flf_vs_empire" ) then
         diff.remove( "flf_vs_empire" )
      end
      if dv_base ~= nil and dv_base:exists() then
         dv_base:rm()
      end
      misn.finish( false )
   end
end
