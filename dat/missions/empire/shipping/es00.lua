--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Empire Shipping 1">
 <unique />
 <priority>2</priority>
 <cond>
   if faction.playerStanding("Empire") &lt; 0 or faction.playerStanding("Dvaered") &lt; 0 or faction.playerStanding("FLF") &gt;= 10 then
      return false
   end
   return require("misn_test").reweight_active()
 </cond>
 <chance>35</chance>
 <done>Soromid Long Distance Recruitment</done>
 <location>Bar</location>
 <faction>Empire</faction>
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
local vn = require "vn"

function create ()
   -- Planet targets
   mem.dest,mem.destsys = spob.getLandable( faction.get("Frontier") )
   mem.ret,mem.retsys   = spob.getS( "Halir" )
   -- Must claim system
   if mem.dest == nil or not misn.claim(mem.destsys) then
      misn.finish(false)
   end

   -- Bar NPC
   misn.setNPC( _("Commander"), emp.soldner.portrait, _("You see an Empire Commander. He seems to have noticed you.") )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local soldner = vn.newCharacter( emp.vn_soldner() )
   vn.transition( emp.soldner.transition )

   -- Intro text
   soldner(fmt.f( _([[You approach the Empire Commander.
"Hello, you must be {player}. I've heard about you from Lieutenant Czesc. I'm Commander Soldner. We've got some harder missions for someone like you in the Empire Shipping division. There would be some real danger involved in these missions, unlike the ones you've recently completed for the division. Would you be up for the challenge?"]]),
      {player=player.name()}))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   vn.done( emp.soldner.transition )

   -- Flavour text and mini-briefing
   vn.label("accept")
   soldner(fmt.f( _([["We've got a prisoner exchange set up with the FLF to take place on {dest_pnt} in the {dest_sys} system. They want a more 'neutral' pilot to do the exchange. You would have to go to {dest_pnt} with some FLF prisoners aboard your ship and exchange them for some of our own. You won't have visible escorts but we will use ships in nearby sectors to monitor your status.
"Once you get our captured people back, bring them over to {ret_pnt} in {ret_sys} for debriefing. You'll be compensated for your troubles. Good luck."]]),
      {dest_pnt=mem.dest, dest_sys=mem.destsys, ret_pnt=mem.ret, ret_sys=mem.retsys} ))
   vn.func( function () accepted = true end )
   vn.na(_([[The prisoners are loaded onto your ship along with a few marines to ensure nothing untoward happens.]]))

   vn.done( emp.soldner.transition )
   vn.run()

   if not accepted then return end

   -- Accept mission
   misn.accept()

   -- target destination
   mem.misn_marker = misn.markerAdd( mem.dest, "low" )

   -- Mission details
   mem.misn_stage = 0
   misn.setTitle(_("Prisoner Exchange"))
   misn.setReward( emp.rewards.es00 )
   misn.setDesc( fmt.f(_("Go to {pnt} in the {sys} system to exchange prisoners with the FLF"), {pnt=mem.dest, sys=mem.destsys}) )
   misn.osdCreate(_("Prisoner Exchange"), {
      fmt.f(_("Go to {pnt} in the {sys} system to exchange prisoners with the FLF"), {pnt=mem.dest, sys=mem.destsys}),
   })
   -- Set up the goal
   local c = commodity.new( N_("Prisoners"), N_("FLF prisoners.") )
   mem.prisoners = misn.cargoAdd( c, 0 )

   -- Set hooks
   hook.land("land")
   hook.enter("enter")
   hook.jumpout("jumpout")
end


function land ()
   mem.landed = spob.cur()
   if mem.landed == mem.dest and mem.misn_stage == 0 then
      misn.cargoRm(mem.prisoners)
      -- Go on to next stage
      mem.misn_stage = 1

      vn.clear()
      vn.scene()
      vn.transition()

      -- Some text
      vn.na(_([[As you land, you notice the starport has been emptied. You also notice explosives rigged on some of the columns. This doesn't look good. The marines tell you to sit still while they go out to try to complete the prisoner exchange.
From the cockpit you see the marines lead the prisoners in front of them with guns to their backs. You see figures step out of the shadows with weapons too; most likely the FLF.]]) )
      vn.music( "snd/sounds/loops/alarm.ogg" ) -- blaring alarm
      vn.na(_([[All of a sudden a siren blares and you hear shooting break out. You quickly start your engines and prepare for take off. Shots ring out all over the landing bay and you can see a couple of corpses as you leave the starport. You remember the explosives just as loud explosions go off behind you. This doesn't look good at all.]]))
      vn.na(_([[You start your climb out of the atmosphere and notice how you're picking up many FLF and Dvaered ships. Looks like you're going to have quite a run to get the hell out of here. This didn't go as you expected.]]) )

      vn.run()

      misn.markerMove( mem.misn_marker, mem.ret )
      misn.setDesc( fmt.f(_("Return to {pnt} in the {sys} system to report what happened."), {pnt=mem.ret, sys=mem.retsys}) )
      misn.osdCreate(_("Prisoner Exchange"), {
         fmt.f(_("Return to {pnt} in the {sys} system to report what happened"), {pnt=mem.ret, sys=mem.retsys}),
      })

      -- Prevent players from saving on the destination planet
      player.allowSave(false)

      -- We'll take off right away again
      player.takeoff()

      -- Saving should be disabled for as short a time as possible
      player.allowSave()
   elseif mem.landed == mem.ret and mem.misn_stage == 1 then

      vn.clear()
      vn.scene()
      local soldner = vn.newCharacter( emp.vn_soldner() )
      vn.transition( emp.soldner.transition )

      -- Flavour text
      vn.na(_([[After you leave your ship in the starport, you meet up with Commander Soldner. From the look on his face, it seems like he already knows what happened.]]))
      soldner(_([["It was all the Dvaered's fault. They just came in out of nowhere and started shooting. What a horrible mess. We're already working on sorting out the blame."]]))
      soldner(_([[He sighs. "We had good people there. And we certainly didn't want you to start with a mess like this, but if you're interested in more missions, meet me up in the bar in a while. We get no rest around here. The payment has already been transferred to your bank account."]]))
      vn.func( function ()
         player.pay( emp.rewards.es00 )
         faction.modPlayerSingle("Empire",5)
      end )
      vn.sfxVictory()
      vn.na(fmt.reward( emp.rewards.es00 ))

      vn.done( emp.soldner.transition )
      vn.run()

      emp.addShippingLog( _([[You took part in a prisoner exchange with the FLF on behalf of the Empire. Unfortunately, the prisoner exchange failed. "It was all the Dvaered's fault. They just came in out of nowhere and started shooting." Commander Soldner has asked you to meet him in the bar on Halir if you're interested in more missions.]]) )

      misn.finish(true)
   end
end

function enter ()
   local sys = system.cur()
   if mem.misn_stage == 1 and sys == mem.destsys then
      -- Force FLF combat music (note: must clear this later on).
      var.push( "music_combat_force", "FLF" )

      -- Get a random position near the player
      local ang = rnd.angle()
      local enter_vect = player.pos() + vec2.newP( rnd.rnd(1500, 2000), ang )

      -- Create some pilots to go after the player
      local flf_sml_force = { "Hyena", "Admonisher", "Vendetta" }
      local p = fleet.add( 1, flf_sml_force, "FLF", enter_vect, _("FLF Ambusher") )

      -- Set hostile
      for k,v in ipairs(p) do
         v:setHostile()
      end

      -- Get a far away position for fighting to happen
      local battle_pos = player.pos() +
            vec2.newP( rnd.rnd(4000, 5000), ang + math.pi )

      -- We'll put the FLF first
      enter_vect = battle_pos + vec2.newP( rnd.rnd(700, 1000), rnd.angle() )
      local flf_med_force = { "Hyena", "Hyena", "Admonisher", "Vendetta", "Pacifier" }
      fleet.add( 1, flf_med_force, "FLF", enter_vect, _("FLF Ambusher") )

      -- Now the Dvaered
      enter_vect = battle_pos + vec2.newP( rnd.rnd(200, 300), rnd.angle() )
      local dv_med_force = { "Dvaered Vendetta", "Dvaered Vendetta", "Dvaered Ancestor", "Dvaered Ancestor", "Dvaered Phalanx", "Dvaered Vigilance" }
      fleet.add( 1, dv_med_force, "Dvaered", enter_vect )

      -- Player should not be able to reland
      player.landAllow(false,_("The docking stabilizers have been damaged by weapons fire!"))
   end
end

function jumpout ()
   -- Storing the system the player jumped from.
   if system.cur() == mem.destsys then
      var.pop( "music_combat_force" )
   end
end

function abort ()
   if system.cur() == mem.destsys then
      var.pop( "music_combat_force" )
   end
   misn.finish(false)
end
