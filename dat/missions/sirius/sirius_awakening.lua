--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Sirius Awakening">
 <unique/>
 <chance>100</chance>
 <cond>
   if faction.playerStanding("Sirius") &lt; 0 then
      return false
   end
   if not var.peek("sirius_awakening") then
      return false
   end
   return true
 </cond>
 <location>Bar</location>
 <faction>Sirius</faction>
</mission>
--]]
local vn = require "vn"
local fmt = require "format"
local vni = require "vnimage"
local flow = require "ships.lua.lib.flow"

-- TODO better portrait? Maybe reusable character?
local talker_portrait, talker_image = vni.sirius.fyrra()
local title = _("Psychic Awakening")
local obelisk, obelisksys = spob.getS("Kal Atok Obelisk")
local target = outfit.get("Seeking Chakra")

function create ()
   misn.setNPC(_("Staring Fyrra"), talker_portrait,
      _("You see a member of the Fyrra echelon that seems to be staring at you."))
end

local function update_osd ()
   local osd = {
      fmt.f(_("Activate the {obelisk} ({sys} system)"),{ obelisk=obelisk, sys=obelisksys })
   }
   if flow.has( player.pilot() ) then
      table.insert( osd, 1, _("Equip a Flow amplifier or Sirius ship") )
   end
   misn.osdCreate( title, osd )
end


function accept ()
   local accepted
   vn.clear()
   vn.scene()
   local f = vn.newCharacter( _("Staring Fyrra"), { image=talker_image } )
   vn.na(_([[The Fyrra echelon individual seems to be clearly staring at you. After you notice, your head starts to throb painfully. You hold on tight to your conscience and manage to not pass out.]]))
   vn.na(_([[Believing the individual to be somehow related to your headaches, you approach them.]]))
   f(_([[The stare at you even as you get close before yelping in astonishment.
"By Sirichana! You have just awakened, haven't you?]]))
   vn.menu{
      {_([["Awakened?"]]), "cont01"},
      {_([["What have you done to me!?"]]), "cont01"},
      {_([[Stare back at them.]]), "cont01"},
   }

   vn.label("cont01")
   f(_([["Sirichana take me! You don't have any idea do you?"]]))
   vn.menu{
      {_([["Stop your word games!"]]), "cont02"},
      {_([["Get to the point!"]]), "cont02"},
      {_([[...]]), "cont02"},
   }

   vn.label("cont02")
   vn.na(_([[They chuckle and stare at you. Your head begins to throb once more. You make an effort to stay on your feet and grab the table, knocking off the drinks which clamour noisely to the ground.]]))
   f(_([["Oops, I guess I overdid it."
They help you up.
"What were you doing? You don't look you are Touched."]]))
   vn.menu{
      {_([["What are you doing to me?!"]]), "cont03"},
      {_([["Is this a game to you?!"]]), "cont03"},
      {_([[Remain silent.]]), "cont03"},
   }

   vn.label("cont03")
   f(_([[They never stop to seem amazed.
"You truly are clueless. I'm not sure where you got your psychic powers, but it seems like you don't either."]]))
   f(_([["You are at least familiar with the House Sirius ways are you not?"]]))
   vn.menu{
      {_([["Psychic powers?"]]), "cont04"},
      {_([["House Siracha?"]]), "cont04_siracha"},
      {_([["Of course!"]]), "cont04"},
      {_([[...]]), "cont04"},
   }

   vn.label("cont04_siracha")
   f(_([[They raise an eyebrow at you.
"You can't be serious, you know you are in House Sirius space right? One of the Great Houses of the Empire?"]]))
   vn.jump("cont04")

   vn.label("cont04")
   f(_([["You may not be aware of it, but House Sirius is famous for our psychic prowess. We are able to project our psyche to manipulate the Flow around us perform all sorts of tasks without moving a muscle!"]]))
   f(_([["Of course it's easier said than done. Psychic powers vary greatly by individual and even then, amplifiers are necessary to show the true powers. Good thing House Sirius ships all have amplifiers builtin."]]))
   f(_([["Here, let me guide you on the basics of Flow."]]))
   vn.na(_([[They teach you what seems to be mainly a serious of breathing exercises that seem like taken out of a relaxation seminar.]]))
   vn.menu{
      {_([["This will make me a psychic?"]]), "cont05_psychic"},
      {_([["I feel more relaxed already."]]), "cont05_relax"},
      {_([[...]]), "cont05"},
   }

   vn.label("cont05_relax")
   f(_([["This will not make you a psychic, but will let you become one. A clear and calm mental state is fundamental for Flow manipulation. Without a sound state of mind, you'll just give yourself headaches, or worse!"]]))
   vn.jump("cont05")

   vn.label("cont05_relax")
   f(_([["You jest, but a clear and calm mental state is fundamental for Flow manipulation. Without a sound state of mind, you'll just give yourself headaches, or worse!"]]))
   vn.jump("cont05")

   vn.label("cont05")
   f(_([["With the basics I just taught you, you should be able to avoid the constant headaches. However, to unlock the full abilities you'll need to equip a Flow amplifier or fly a House Sirius ship."]]))
   f(fmt.f(_([["By Sirichana! I almost forgot. To truly master Flow, you have to tune into Sirichana's Obelisks. It has been ages since I tried their challenges, but I can mark the {spob} on your map if you wish to endeavour in learning to control Flow."]]),{spob=obelisk}))
   vn.menu{
      {_([[Get the Obelisk coordinates.]]), "cont06_obelisk"},
      {_([[Pass on the headache stuff.]]), "cont06_pass"},
   }

   vn.label("cont06_pass")
   f(_([["Are you sure? You can still find it later if you are so inclined, but this would be much easier."]]))
   vn.menu{
      {_([[Get the Obelisk coordinates.]]), "cont06_obelisk"},
      {_([[Not interested in more headaches.]]), "cont06_pass2"},
   }

   vn.label("cont06_pass2")
   f(_([["Oh well, I'm sure you'll turn around. May Sirichana guide you."]]))
   vn.na(_([[You step away from them not wanting anything to do with more headaches. It may still be worth looking into the Obelisks if you feel so inclined.]]))
   vn.done()

   vn.label("cont06_obelisk")
   vn.func( function() accepted = true end )
   f(fmt.f(_([[They joyfully give you the coordinates to the {spob}.
"Make sure to have a Flow amplifier equipped, be it an external outfit or a built-in House Sirius ship one, and use the exercises I taught you to tune into the obelisk. You should be able to figure out the rest from there. May Sirichana guide you."]]),{spob=obelisk}))
   vn.na(_([[You take your leave and hope you haven't bitten off more than you can chew.]]))

   vn.run()

   -- Our player is now officially a "psychic"
   var.push("sirius_psychic",true)

   if not accepted then
      -- Not interested in mission, so we just finish
      misn.finish(true)
      return
   end

   misn.accept()

   misn.setTitle(title)
   misn.setReward(fmt.f(_("The {outfit} flow ability."),{outfit=target}))
   misn.setDesc( fmt.f(_("Go to the {sys} system and activate the {obelisk} and complete the trial."),
      {obelisk=obelisk, sys=obelisksys}) )
   update_osd()

   misn.markerAdd( obelisk )

   hook.enter( "enter" )
end

function enter ()
   -- Finished when player gets the outfit (successfully completes obelisk)
   if player.outfitNum( target ) > 0 then
      misn.finish(true)
      return
   end

   -- Just update every time player enters
   update_osd()
end
