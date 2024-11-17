--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Android Abroad">
 <unique/>
 <location>Bar</location>
 <chance>50</chance>
 <faction>Thurion</faction>
 <cond>system.cur():reputation("Thurion") &gt;= 0</cond>
</mission>
--]]
--[[
   Just used to create an OSD. Doesn't really affect anything.
--]]
local fmt = require "format"
local vn = require "vn"
local title = _("Android Abroad")

local destspb, destsys = spob.getS("Tenal-P")
-- TODO non-generic portrait?
local npc_image = "neutral/male1n.webp"
local npc_portrait = "neutral/male1n.webp"

local cargo_amount = 10
local reward = 200e3

function create ()
   -- Note: this mission does not make any system claims.

   misn.setNPC(_("Odd Individual"), npc_portrait, _("You see a non-uploaded Thurion sitting at the bar."))
end

local talked
function accept ()
   local accepted = false
   vn.clear()
   vn.scene()
   local liao = vn.newCharacter( _("Liao"), {image=npc_image} )
   vn.transition()

   if talked then
      liao(fmt.f(_([["Are you able to help me get to {destspb} in the {destsys} system?"]]),
         {destspb=destspb, destsys=destsys}))
   else
      vn.na(_([[As you approach the individual you sort of feel something odd. There seems to be something uncanny about them.]]))
      liao(fmt.f(_([["Hello, {player}. Nice to meet you, my name is Liao."]]),
         {player=player.name()}))
      vn.menu{
         {_([["How did you know my name?"]]), "01_name"},
         {_([["Nice to meet you."]]), "01_meet"},
         {_([[...]]), "01_cont"},
      }

      vn.label("01_name")
      liao(_([["We Thurion are always in track of everything. It helps to be connected with everyone all the time."]]))
      vn.jump("01_cont")

      vn.label("01_meet")
      liao(_([["I'm very intrigued by you. It's not often we get visitors here, and much less voluntary ones."]]))
      vn.jump("01_cont")

      vn.label("01_cont")
      liao(_([["I see, you don't know what to make of me. Usually nobody gives me a second glance, but it seems like you've noticed."]]))
      liao(_([[They poke their cheek.
"See this? All synthetics. We are testing our newest android models, only one in a thousand people can see through them, and only if they focus. Most give a second glance. I am just temporarily downloaded into this body. Reminds me of before I was uploaded, but I get to keep my connection with the rest of the Thurion."]]))
      liao(_([["Enough about me, I have a proposal for you. We're beginning some new intelligence operations outside the Nebula, and we could use your expertise on the topic."]]))
      liao(fmt.f(_([["Up until now, we've been getting most of our information second hand, and that only goes so far. I would need you to take me and some equipment to..."
Their eyes flicker for a second.
"...{destspb} in the {destsys} system. Would you be able to help with such a simple task?"]]),
         {destspb=destspb, destsys=destsys}))
   end
   vn.menu{
      {_([[Accept.]]), "accept"},
      {_([[Decline for now.]]), "decline"},
   }

   vn.label("decline")
   liao(_([["I'll be waiting if you change your mind."]]))
   vn.done()

   vn.label("nospace")
   vn.na(fmt.f(_([[You need at least {cargo} of free space to accept this mission.]]),
      {cargo=fmt.tonnes(cargo_amount)}))
   vn.done()

   vn.label("accept")
   vn.func( function ()
      if player.fleetCargoMissionFree() < cargo_amount then
         return vn.jump("nospace")
      end
      accepted = true
   end )
   liao(_([["We're always glad to have more people support the Thurion and understand our ways. Let me load my things on your ship and let us be on our way."]]))

   vn.run()
   talked = true

   if not accepted then return end

   misn.accept()
   misn.setTitle(title)
   misn.setDesc(fmt.f(_([[You have been asked by Liao to take them to {spb} ({sys} system).]]),
      {spb=destspb, sys=destsys}))
   misn.setReward( reward )
   misn.osdCreate( title, {
      fmt.f(_([[Land on {spb} ({sys} system)]]), {spb=destspb, sys=destsys}),
   } )

   local c = commodity.new( N_("Liao"), N_("A Thurion android named Liao and a lot of equipment. Seems like a bunch of computation machines.") )
   mem.carg_id = misn.cargoAdd(c, cargo_amount)
   misn.markerAdd( destspb )

   hook.land("land")
end

function land ()
   if spob.cur()~=destspb then return end

   vn.clear()
   vn.scene()
   local liao = vn.newCharacter( _("Liao"), {image=npc_image} )
   vn.transition()

   vn.na(fmt.f(_([[You land on the tiny {spb}, and drop off Liao and their equipment.]]),
      {spb=destspb}))
   liao(_([["Thank you for the pleasant journey. It has been a while since I've been outside the Nebula."]]))
   vn.menu{
      {_([["Are you going to be alright?"]]), "01_alright"},
      {_([["What's next?"]]), "01_cont"},
   }

   vn.label("01_alright")
   liao(_([["Do not worry about me, I have arrangements and don't expect any difficulties here. However, there is a lot of work left to do."]]))
   vn.jump("01_cont")

   vn.label("01_cont")
   liao(fmt.f(_([["Before I send you off your way, let me hook you up with a special code for the Mission Bulletin Board System, you'll be able to find small tasks for us. However, they won't be as easy as this one. For now, we only operate through around here, and {spb}. However, we hope that will change in the future."]]),
      {spb=spob.get("Cerberus Outpost")}))
   liao(_([[They sort of blank out for a second, before suddenly coming back.
"Sorry, I haven't been this disconnected in a while. Even though I trained for it, it will take some getting used to. Here, take a credit stick and let us meet again, somewhere, someday."]]))
   vn.na(_([[They flash a smile and head off towards the interior of the station full of confidence.]]))

   vn.sfxVictory()
   vn.func( function() player.pay(reward) end )
   vn.na( fmt.reward(reward) )

   diff.apply("thurion_espionage")
   faction.hit( "Thurion", 3 )

   vn.run()

   misn.finish(true)
end
