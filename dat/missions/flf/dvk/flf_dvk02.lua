--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="FLF Pirate Alliance">
 <unique />
 <priority>2</priority>
 <chance>30</chance>
 <done>Diversion from Raelid</done>
 <location>Bar</location>
 <faction>FLF</faction>
 <cond>spob.cur():reputation("FLF") &gt;= 30</cond>
 <notes>
  <campaign>Save the Frontier</campaign>
 </notes>
</mission>
--]]
--[[

   Pirate Alliance

--]]
local fmt = require "format"
local fleet = require "fleet"
local flf = require "common.flf"

local boss, pirates -- Non-persistent state

-- TODO this mission needs to be adapted to the new pirate clan stuff
-- for now I just swapped "Pirate" for "Dreamer Clan"

local comm_boss_insults = {}
comm_boss_insults[1] = _("You call those weapons? They look more like babies' toys to me!")
comm_boss_insults[2] = _("What a hopeless weakling!")
comm_boss_insults[3] = _("What, did you really think I would be impressed that easily?")
comm_boss_insults[4] = _("Keep hailing all you want, but I don't listen to weaklings!")
comm_boss_insults[5] = _("We'll have your ship plundered in no time at all!")

mem.osd_desc    = {}
mem.osd_desc[2] = _("Find pirates and try to talk to (hail) them")

mem.osd_desc[3] = _("Return to FLF base")

function create ()
   mem.missys = system.get( "Tormulex" )
   mem.missys2 = system.get( "Anger" )
   if not misn.claim( mem.missys ) then
      misn.finish( false )
   end

   misn.setNPC( _("Benito"), "flf/unique/benito.webp", _("It seems Benito wants something from you again. Something about her looks a little off this time around.") )
end


function accept ()
   tk.msg( _("The enemy of my enemy..."), fmt.f( _([[Benito motions for you to sit. She doesn't seem quite as calm and relaxed as she usually is.
   "Hello again, {player}. Look: we have a pretty bad situation here. As you may have guessed, we rely on... unconventional sources for supplies. Unfortunately, we seem to have hit a bit of a snag. See, one of our important sources has stopped supplying us, and I fear we may be cut off and no longer able to carry out our operations before long if we don't do something.
   "But that being said, I think I may have found a solution. See, we have reason to believe that we are actually neighbouring a pirate stronghold. We're not entirely sure, but we have detected some evidence of occasional pirate activity in the nearby {sys} system."]]), {player=player.name(), sys=mem.missys} ) )
   if tk.yesno( _("The enemy of my enemy..."), fmt.f( _([[You raise an eyebrow. It seems rather odd that pirates would be in such a remote system. Perhaps it could be a gateway of some sort?
   "You must be thinking the same thing," Benito pipes up. "Yes, that is a very strange system to see pirates in, even occasionally. That's why we think there is a secret pirate stronghold nearby. It may even be the one associated with piracy in the Frontier.
   "We must establish trading relations with that stronghold at once. This could give us just the edge we need against the Dvaereds. I honestly don't know how you can go about doing it, but my recommendation would be to go to the {sys} system and see if you find any pirates. Tell them you're on official FLF business, and that we're seeking to become trade partners with them. Are you in?"]]), {sys=mem.missys} ) ) then
      tk.msg( _("...is my friend."), fmt.f( _([["Excellent! I knew you would do it." Benito becomes visibly more relaxed, almost her usual self. "Now, {player}, I'm sure you're well aware of this, but please remember that pirates are extremely dangerous. They will probably attack you, and they may have demands. I'm counting on you to overcome any... obstacles you may encounter and secure a deal." You nod in understanding. "Good," she says. "Report back here with your results." Benito then excuses herself, presumably to take care of other things.]]), {player=player.name()} ) )

      misn.accept()

      mem.osd_desc[1] = fmt.f( _("Fly to the {sys} system"), {sys=mem.missys} )
      misn.osdCreate( _("Pirate Alliance"), mem.osd_desc )
      misn.setTitle( _("Pirate Alliance") )
      misn.setDesc( fmt.f( _("You are to seek out pirates in the {sys} system and try to convince them to become trading partners with the FLF."), {sys=mem.missys} ) )
      mem.marker = misn.markerAdd( mem.missys, "plot" )
      misn.setReward( _("Supplies for the FLF") )

      mem.stage = 0
      mem.pirates_left = 0
      mem.boss_hailed = false
      mem.boss_impressed = false
      boss = nil
      pirates = nil
      mem.boss_hook = nil

      mem.ore_needed = 40
      mem.credits = 300e3
      mem.reputation = 1
      mem.pir_reputation = 10
      mem.pir_starting_reputation = faction.get("Dreamer Clan"):reputationGlobal()

      hook.enter( "enter" )
   else
      tk.msg( _("...is still my enemy."), _([["That's too bad. I understand where you're coming from, though. Please feel free to return if you are willing to take on this mission at a later date."]]) )
   end
end


function pilot_hail_pirate ()
   player.commClose()
   if mem.stage <= 1 then
      player.msg( _("Har, har, har! You're hailing the wrong ship, buddy. Latest word from the boss is you're a weakling just waiting to be plundered!") )
   else
      player.msg( _("I guess you're not so bad after all!") )
   end
end


function pilot_hail_boss ()
   player.commClose()
   if mem.stage <= 1 then
      if mem.boss_impressed then
         mem.stage = 2
         local standing = faction.get("Dreamer Clan"):reputationGlobal()
         if standing < 25 then
            faction.get("Dreamer Clan"):setReputationGlobal( 25 )
         end

         if boss ~= nil then
            boss:changeAI( "pirate" )
            boss:setHostile( false )
            boss:setFriendly()
         end
         if pirates ~= nil then
            for i, j in ipairs( pirates ) do
               if j:exists() then
                  j:changeAI( "pirate" )
                  j:setHostile( false )
                  j:setFriendly()
               end
            end
         end

         tk.msg( _("Not So Weak After All"), _([[The pirate comes on your view screen once again, but his expression has changed this time. He's hiding it, but you can tell that he's afraid of what you might do to him. You come to the realization that he is finally willing to talk and suppress a sigh of relief.
   "L-look, we got off on the wrong foot, eh? I've misjudged you lot. I guess FLF pilots can fight after all."]]) )
         tk.msg( _("Not So Weak After All"), fmt.f( _([[You begin to talk to the pirate about what you and the FLF are after, and the look of fear on the pirate's face fades away. "Supplies? Yeah, we've got supplies, alright. But it'll cost you! Heh, heh, heh..." You inquire as to what the cost might be. "Simple, really. We want to build another base in the {sys} system. We can do it ourselves, of course, but if we can get you to pay for it, even better! Specifically, we need another {tonnes} of ore to build the base. So you bring it back to the Anger system, and we'll call it a deal!
   "Oh yeah, I almost forgot; you don't know how to get to the Anger system, now, do you? Well, since you've proven yourself worthy, I suppose I'll let you in on our little secret." He transfers a file to your ship's computer. When you look at it, you see that it's a map showing a single hidden jump point. "Now, away with you! Meet me in the {sys} system when you have the loot."]]),
            {sys=mem.missys2, tonnes=fmt.tonnes(mem.ore_needed)} ) )

         player.outfitAdd( "Map: FLF-Pirate Route" )
         if mem.marker ~= nil then misn.markerRm( mem.marker ) end
         mem.marker = misn.markerAdd( mem.missys2, "plot" )

         mem.osd_desc[4] = fmt.f( _("Bring {tonnes} of Ore to the Pirate Kestrel in the {sys} system"), {tonnes=fmt.tonnes(mem.ore_needed), sys=mem.missys2} )
         mem.osd_desc[5] = _("Return to FLF base")
         misn.osdCreate( _("Pirate Alliance"), mem.osd_desc )
         misn.osdActive( 4 )
      else
         if mem.boss_hailed then
            player.msg( comm_boss_insults[ rnd.rnd( 1, #comm_boss_insults ) ] )
         else
            mem.boss_hailed = true
            if mem.stage <= 0 then
               tk.msg( _("Who are you calling a weakling?"), _([[A scraggly-looking pirate appears on your viewscreen. You realize this must be the leader of the group. "Bwah ha ha!" he laughs. "That has to be the most pathetic excuse for a ship I've ever seen!" You try to ignore his rude remark and start to explain to him that you just want to talk. "Talk?" he responds. "Why would I want to talk to a normie like you? Why, I'd bet my mates right here could blow you out of the sky even without my help!"
   The pirate immediately cuts his connection. Well, if these pirates won't talk to you, maybe it's time to show him what you're made of. Destroying just one or two of his escorts should do the trick.]]) )
               mem.osd_desc[3] = _("Destroy some of the weaker pirate ships, then try to hail the Kestrel again")
               mem.osd_desc[4] = _("Return to FLF base")
               misn.osdCreate( _("Pirate Alliance"), mem.osd_desc )
               misn.osdActive( 3 )
            else
               tk.msg( _("Still Not Impressed"), _([[The pirate leader comes on your screen once again. "Lucky shot, normie!" he says before promptly terminating the connection once again. Perhaps you need to destroy some more of his escorts so he can see you're just a bit more than a "normie".]]) )
            end
         end
      end
   elseif player.pilot():cargoHas( "Ore" ) >= mem.ore_needed then
      tk.msg( _("I knew we could work something out"), _([["Ha, you came back after all! Wonderful. I'll just take that ore, then." You hesitate for a moment, but considering the number of pirates around, they'll probably take it from you by force if you refuse at this point. You jettison the cargo into space, which the Kestrel promptly picks up with a tractor beam. "Excellent! Well, it's been a pleasure doing business with you. Send your mates over to the new station whenever you're ready. It should be up and running in just a couple periods or so. And in the meantime, you can consider yourselves one of us! Bwa ha ha!"
   You exchange what must, for lack of a better word, be called pleasantries with the pirate, with him telling a story about a pitifully armed Mule he recently plundered and you sharing stories of your victories against Dvaered scum. You seem to get along well. You then part ways. Now to report to Benito....]]) )
      mem.stage = 3
      player.pilot():cargoRm( "Ore", mem.ore_needed )
      hook.rm( mem.boss_hook )
      hook.land( "land" )
      misn.osdActive( 5 )
      if mem.marker ~= nil then misn.markerRm( mem.marker ) end
   else
      player.msg( _("Don't be bothering me without the loot, you hear?") )
   end
end


function pilot_death_pirate ()
   if mem.stage <= 1 then
      mem.pirates_left = mem.pirates_left - 1
      mem.stage = 1
      mem.boss_hailed = false
      if mem.pirates_left <= 0 or rnd.rnd() < 0.25 then
         mem.boss_impressed = true
      end
   end
end


function pilot_death_boss ()
   tk.msg( _("Mission Failure"), _([[As the Pirate Kestrel is blown out of the sky, it occurs to you that you have made a terrible mistake. Having killed off the leader of the pirate group, you have lost your opportunity to negotiate a trade deal with the pirates. You shamefully transmit your result to Benito via a coded message and abort the mission. Perhaps you will be given another opportunity later.]]) )
   misn.finish( false )
end


function enter ()
   if mem.stage <= 1 then
      mem.stage = 0
      if system.cur() == mem.missys then
         pilot.clear()
         pilot.toggleSpawn( false )
         local r = system.cur():radius()
         local vec = vec2.new( rnd.rnd( -r, r ), rnd.rnd( -r, r ) )

         boss = pilot.add( "Pirate Kestrel", "Dreamer Clan", vec, nil, {ai="pirate_norun"} )
         hook.pilot( boss, "death", "pilot_death_boss" )
         hook.pilot( boss, "hail", "pilot_hail_boss" )
         boss:setHostile()
         boss:setHilight()

         mem.pirates_left = 4
         pirates = fleet.add( mem.pirates_left, "Pirate Hyena", "Dreamer Clan", vec, nil, {ai="pirate_norun"} )
         for i, j in ipairs( pirates ) do
            hook.pilot( j, "death", "pilot_death_pirate" )
            hook.pilot( j, "hail", "pilot_hail_pirate" )
            j:setHostile()
         end

         misn.osdActive( 2 )
      else
         mem.osd_desc[3] = _("Return to FLF base")
         mem.osd_desc[4] = nil
         misn.osdCreate( _("Pirate Alliance"), mem.osd_desc )
         misn.osdActive( 1 )
      end
   elseif mem.stage <= 2 then
      if system.cur() == mem.missys2 then
         local r = system.cur():radius()
         local vec = vec2.new( rnd.rnd( -r, r ), rnd.rnd( -r, r ) )

         boss = pilot.add( "Pirate Kestrel", "Dreamer Clan", vec, nil, {ai="pirate_norun"} )
         hook.pilot( boss, "death", "pilot_death_boss" )
         mem.boss_hook = hook.pilot( boss, "hail", "pilot_hail_boss" )
         boss:setFriendly()
         boss:setHilight()
         boss:setVisible()
      end
   end
end


function land ()
   if mem.stage >= 3 and spob.cur():faction() == faction.get( "FLF" ) then
      tk.msg( _("Just The Edge We Need"), fmt.f( _([[You greet Benito in a friendly manner as always, sharing your story and telling her the good news before handing her a chip with the map data on it. She seems pleased. "Excellent," she says. "We'll begin sending our trading convoys out right away. We'll need lots of supplies for our next mission! Thank you for your service, {player}. Your pay has been deposited into your account. It will be a while before we'll be ready for your next big mission, so you can do some missions on the mission computer in the meantime. And don't forget to visit the Pirate worlds yourself and bring your own ship up to par!
   "Oh, one last thing. Make sure you stay on good terms with the pirates, yeah? The next thing you should probably do is buy a Skull and Bones ship; pirates tend to respect those who use their ships more than those who don't. And make sure to destroy Dvaered scum with the pirates around! That should keep your reputation up." You make a mental note to do what she suggests as she excuses herself and heads off.]]), {player=player.name()} ) )
      diff.apply( "Fury_Station" )
      diff.apply( "flf_pirate_ally" )
      player.pay( mem.credits )
      flf.setReputation( 50 )
      faction.get("FLF"):hit( mem.reputation )
      faction.get("Dreamer Clan"):hit( mem.pir_reputation )
      flf.addLog( _([[You helped the Pirates to build a new base in the Anger system and established a trade alliance between the FLF and the Pirates. Benito suggested that you should buy a Skull and Bones ship from the pirates and destroy Dvaered ships in areas where pirates are to keep your reputation with the pirates up. She also suggested you may want to upgrade your ship now that you have access to the black market.]]) )
      misn.finish( true )
   end
end


function abort ()
   faction.get("Dreamer Clan"):setReputationGlobal( mem.pir_starting_reputation )
   local hj1, hj2 = jump.get( "Tormulex", "Anger" )
   hj1:setKnown( false )
   hj2:setKnown( false )
   misn.finish( false )
end
