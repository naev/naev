--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Empire Shipping 1">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>2</priority>
  <cond>faction.playerStanding("Empire") &gt;= 0 and faction.playerStanding("Dvaered") &gt;= 0 and faction.playerStanding("FLF") &lt; 10</cond>
  <chance>35</chance>
  <done>Frontier Long Distance Recruitment</done>
  <location>Bar</location>
  <faction>Empire</faction>
 </avail>
 <notes>
  <campaign>Empire Shipping</campaign>
  <tier>2</tier>
 </notes>
</mission>
--]]
--[[

   Empire Shipping Prisoner Exchange

   Author: bobbens
      minor edits by Infiltrator

]]--
local fleet = require "fleet"
local fmt = require "format"
local emp = require "common.empire"

text = {}
text[1] = _([[You approach the Empire Commander.
    "Hello, you must be %s. I've heard about you. I'm Commander Soldner. We've got some harder missions for someone like you in the Empire Shipping division. There would be some real danger involved in these missions, unlike the ones you've recently completed for the division. Would you be up for the challenge?"]])
text[2] = _([["We've got a prisoner exchange set up with the FLF to take place on %s in the %s system. They want a more neutral pilot to do the exchange. You would have to go to %s with some FLF prisoners aboard your ship and exchange them for some of our own. You won't have visible escorts but we will have your movements watched by ships in nearby sectors.
    "Once you get the men they captured back, bring them over to %s in %s for debriefing. You'll be compensated for your troubles. Good luck."]])
text[3] = _([[The Prisoners are loaded onto your ship along with a few marines to ensure nothing untoward happens.]])
text[4] = _([[As you land, you notice the starport has been emptied. You also notice explosives rigged on some of the columns. This doesn't look good. The marines tell you to sit still while they go out to try to complete the prisoner exchange.
    From the cockpit you see how the marines lead the prisoners in front of them with guns to their backs. You see figures step out of the shadows with weapons too; most likely the FLF.]])
text[5] = _([[All of a sudden a siren blares and you hear shooting break out. You quickly start your engines and prepare for take off. Shots ring out all over the landing bay and you can see a couple of corpses as you leave the starport. You remember the explosives just as loud explosions go off behind you. This doesn't look good at all.
    You start your climb out of the atmosphere and notice how you're picking up many FLF and Dvaered ships. Looks like you're going to have quite a run to get the hell out of here. It didn't go as you expected.]])
text[6] = _([[After you leave your ship in the starport, you meet up with Commander Soldner. From the look on his face, it seems like he already knows what happened.
    "It was all the Dvaered's fault. They just came in out of nowhere and started shooting. What a horrible mess. We're already working on sorting out the blame."
    He sighs. "We had good men there. And we certainly didn't want you to start with a mess like this, but if you're interested in another, meet me up in the bar in a while. We get no rest around here. The payment has already been transferred to your bank account."]])

function create ()
   -- Target destination
   dest,destsys = planet.getLandable( faction.get("Frontier") )
   ret,retsys   = planet.getLandable( "Halir" )
   if dest == nil or ret == nil or not misn.claim(destsys) then
      misn.finish(false)
   end

   -- Spaceport bar stuff
   misn.setNPC( _("Commander"), "empire/unique/soldner.webp", _("You see an Empire Commander. He seems to have noticed you.") )
end


function accept ()
   -- Intro text
   if not tk.yesno( _("Spaceport Bar"), string.format( text[1], player.name() ) ) then
      misn.finish()
   end

   -- Accept mission
   misn.accept()

   -- target destination
   misn_marker = misn.markerAdd( destsys, "low" )

   -- Mission details
   misn_stage = 0
   reward = 500e3
   misn.setTitle(_("Prisoner Exchange"))
   misn.setReward( fmt.credits(reward) )
   misn.setDesc( string.format(_("Go to %s in the %s system to exchange prisoners with the FLF"), dest:name(), destsys:name()))

   -- Flavour text and mini-briefing
   tk.msg( _("Prisoner Exchange"), string.format( text[2], dest:name(), destsys:name(),
         dest:name(), ret:name(), retsys:name() ))
   misn.osdCreate(_("Prisoner Exchange"), {_("Go to %s in the %s system to exchange prisoners with the FLF"):format(dest:name(),destsys:name())})
   -- Set up the goal
   local c = misn.cargoNew( N_("Prisoners"), N_("FLF prisoners.") )
   prisoners = misn.cargoAdd( c, 0 )
   tk.msg( _("Prisoner Exchange"), text[3] )

   -- Set hooks
   hook.land("land")
   hook.enter("enter")
   hook.jumpout("jumpout")
end


function land ()
   landed = planet.cur()
   if landed == dest and misn_stage == 0 then
      if misn.cargoRm(prisoners) then
         -- Go on to next stage
         misn_stage = 1

         -- Some text
         tk.msg(_("Prisoner Exchange"), text[4] )
         tk.msg(_("Prisoner Exchange"), text[5] )
         misn.markerMove( misn_marker, retsys )
         misn.setDesc( string.format(_("Return to %s in the %s system to report what happened"), ret:name(), retsys:name()))
         misn.osdCreate(_("Prisoner Exchange"), {_("Return to %s in the %s system to report what happened"):format(ret:name(),retsys:name())})

         -- Prevent players from saving on the destination planet
         player.allowSave(false)

         -- We'll take off right away again
         player.takeoff()

         -- Saving should be disabled for as short a time as possible
         player.allowSave()
      end
   elseif landed == ret and misn_stage == 1 then

      -- Rewards
      player.pay(reward)
      faction.modPlayerSingle("Empire",5);

      -- Flavour text
      tk.msg(_("Mission Report"), text[6] )

      emp.addShippingLog( _([[You took part in a prisoner exchange with the FLF on behalf of the Empire. Unfortunately, the prisoner exchange failed. "It was all the Dvaered's fault. They just came in out of nowhere and started shooting." Commander Soldner has asked you to meet him in the bar on Halir if you're interested in another mission.]]) )

      misn.finish(true)
   end
end


function enter ()
   local sys = system.cur()
   if misn_stage == 1 and sys == destsys then
      -- Force FLF combat music (note: must clear this later on).
      var.push( "music_combat_force", "FLF" )

      -- Get a random position near the player
      ang = rnd.rnd(0, 360)
      enter_vect = player.pos() + vec2.newP( rnd.rnd(1500, 2000), ang )

      -- Create some pilots to go after the player
      local flf_sml_force = { "Hyena", "Admonisher", "Vendetta" }
      p = fleet.add( 1, flf_sml_force, "FLF", enter_vect, _("FLF Ambusher") )

      -- Set hostile
      for k,v in ipairs(p) do
         v:setHostile()
      end

      -- Get a far away position for fighting to happen
      local battle_pos = player.pos() +
            vec2.newP( rnd.rnd(4000, 5000), ang + 180 )

      -- We'll put the FLF first
      enter_vect = battle_pos + vec2.newP( rnd.rnd(700, 1000), rnd.rnd(0, 360) )
      local flf_med_force = { "Hyena", "Hyena", "Admonisher", "Vendetta", "Pacifier" }
      fleet.add( 1, flf_med_force, "FLF", enter_vect, _("FLF Ambusher") )

      -- Now the Dvaered
      enter_vect = battle_pos + vec2.newP( rnd.rnd(200, 300), rnd.rnd(0, 360) )
      local dv_med_force = { "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Ancestor", "Dvaered Ancestor", "Dvaered Phalanx", "Dvaered Vigilance" }
      fleet.add( 1, dv_med_force, "Dvaered", enter_vect )

      -- Player should not be able to reland
      player.allowLand(false,_("The docking stabilizers have been damaged by weapons fire!"))
   end
end


function jumpout ()
   -- Storing the system the player jumped from.
   if system.cur() == destsys then
      var.pop( "music_combat_force" )
   end
end


function abort ()
   if system.cur() == destsys then
      var.pop( "music_combat_force" )
   end
   misn.finish(false)
end
