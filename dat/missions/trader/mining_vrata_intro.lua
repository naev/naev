--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Mining Vrata Intro">
 <unique/>
 <priority>4</priority>
 <chance>5</chance>
 <location>Bar</location>
 <cond>
   require("misn_test").cargo()
 </cond>
 <tags>
  <tag>trader_cap_medium</tag>
 </tags>
</mission>
--]]
local trader = require "common.trader"
local fmt = require "format"
local vn = require "vn"
local vni = require "vnimage"

local TITLE    = _("Help the Mining Vrata")
local NEEDED   = 10 -- amount needed
local FACTION  = faction.get("Traders Society")
local REWARD   = 100e3

local NPCNAME  = _("Mining Vrata Employee")
local npcvn, npcpor = vni.generic()
function create ()
   misn.setNPC( NPCNAME, npcpor, _("Ther eseems to be a Mining Vrata employee looking for pilots willing to work.") )
   misn.setTitle( TITLE )
   misn.setReward( REWARD )
   misn.setDesc(fmt.f(_([[You have been asked to obtain {amount} of uncommon or rare minerals from mining asteroid fields, and deliver them to a {faction} planet or station.]]), {
      amount   = NEEDED,
      faction  = FACTION,
   }))
end

function accept ()
   local accepted

   vn.reset()
   vn.scene()
   local npc = vn.newCharacter( NPCNAME, { image=npcvn } )
   vn.transition()

   -- Strike the earth is from dwarf fortress.
   npc(_([["Hey, the Mining Vrata is looking for more miners. You ready to do some deep space mining and 'strike the earth'?]]))
   vn.menu{
      { _("Mining Vrata?"), "01_mining_vrata" },
      { _("Accept."), "01_accept" },
      { _("Maybe later."), "01_later" },
   }

   vn.label("01_mining_vrata")
   npc(_([["You haven't heard of one of the biggest guilds of the Space Trader's Society? The Mining Vrata consists of all sort of individuals and companies interested in deep space mining. We keep civilization ticking!"]]))
   npc(_([["What do you say, are you interested in mining for the Mining Vrata?"]]))
   vn.menu{
      { _("Accept."), "01_accept" },
      { _("Maybe later."), "01_later" },
   }

   vn.label("01_accept")
   vn.func( function ()
      accepted = true
   end )
   npc(_([["Great! Wait, one second, you do know how to mine right?"]]))
   vn.menu{
      { _("Teach me."), "01_teach" },
      { _("Leave it to me!"), "01_miner" },
   }

   vn.label("01_teach")
   npc(_([["Wanting to try new things, am I right? OK, so mining space asteroids is fairly simple. It's a 3 step process. First you find the asteroids, then you mine them! Wait, isn't that 2 steps?"]]))
   npc(_([["Anyway, first thing you have to do is find some asteroids. You can see that on the map where a system will indicate whether it has asteroids or not when selected. However, not all asteroid fields are equal. Some have only common minerals, while other ones will have uncommon, and the best will have rare minerals!"]]))
   npc(_([["In general, #basteroid fields that are harder to get to will have more rare minerals#0! You know, if it's easy to get to, it'll likely be depleted by miner trainees and the likes. So you've got to get a bit away from civilization to find the real good stuff. You'll need to get an #basteroid scanner#0 if you want to see what each individual asteroid contains."]]))
   npc(_([["Next step is the fun part, blowing it up. Of course, you can use any weapon, but that doesn't get you the best minerals out of an asteroid. To get uncommon minerals, you'll need #bmining lances#0, but for rare minerals, #bmining drills#0 is the best to get them out safely. Mining drills also come with built-in asteroid scanners, so they are a great solution for mining. Then it's just a question of collecting the materials."]]))
   npc(_([["You can always find more detailed information in the holo-archives. Was that clear?"]]))
   vn.menu{
      { _([["Yup."]]), "01_miner" },
      { _([["Once more."]]), "01_teach" },
   }

   vn.label("01_miner")
   npc(fmt.f(_([["OK! I'll mark down your info into the Mining Vrata database. Whenever you mine {amount} of uncommon or rare minerals, just deliver them to the nearest {faction} planet or station. They will be marked on your map. Will to drill!"]]), {
      amount   = NEEDED,
      faction  = FACTION,
   } ))

   vn.label("01_later")
   vn.run()

   if not accepted then return end

   misn.accept()

   misn.osdCreate( TITLE, {
      fmt.f(_("Mine at least {amount} of an uncommon or rare mineral"), {
         amount = fmt.tonnes(NEEDED),
      } ),
      fmt.f( _("Deliver the cargo to a {faction} planet or station"), {
         faction = FACTION,
      } ),
   } )

   hook.land( "land" )
   hook.gather( "update" )
   hook.enter( "update" )
end

local function valid_cargos()
   local cargos = {}
   for k,v in ipairs( player.fleetCargoList() ) do
      local t =v.c:tags()
      -- Don't want to handle special for now
      if t.mining and v.q>=NEEDED and (t.uncommon or t.rare) then
      --if t.mining and v.q>=NEEDED and (t.uncommon or t.rare or t.special) then
         if t.uncommon then
            v.t = _("uncommon")
         elseif t.rare then
            v.t = "#o".._("rare").."#0"
            v.rare = true
         --elseif t.special then
         --   v.t = "#r".._("special").."#0"
         end
         table.insert( cargos, v )
      end
   end
   return cargos
end

-- Handles updating the OSD goal
function update ()
   local cargos = valid_cargos()
   if #cargos > 0 then
      if misn.osdGetActive()~=2 then
         misn.osdActive( 2 )
         misn.markerRm()
         for k,s in ipairs(spob.getAll()) do
            if s:faction()==FACTION then
               misn.markerAdd( s )
            end
         end
      end
   else
      if misn.osdGetActive()~=1 then
         misn.osdActive( 1 )
         misn.markerRm()
      end
   end
end

function land ()
   local cargos = valid_cargos()
   if #cargos < 0 then return end

   local cspb = spob.cur()
   if cspb:faction() ~= FACTION then return end

   vn.reset()
   vn.scene()
   vn.transition()

   vn.na(fmt.f(_([[Do you want to hand in {amount} of uncommon or rare ores to the Mining Vrata?]]), {
      amount = NEEDED,
   } ) )
   local present
   vn.menu( function ()
      local opts = { { _("Not yet."), "01_later" } }
      for k,v in ipairs(cargos) do
         table.insert( opts, 1, { fmt.f(_([[Hand in {name} ({type}, you have {amount})]]), {
            name     = v.c,
            type     = v.t,
            amount   = fmt.tonnes(v.q),
         } ), v } )
      end
      return opts
   end, function ( val )
      if type(val)=="string" then
         return vn.jump(val)
      end
      present = val
      vn.jump("01_present")
   end )

   vn.label("01_later")
   vn.done()

   vn.label("01_present")
   vn.na( function ()
      player.fleetCargoRm( present.c, NEEDED )
      return fmt.f(_([[You hand in the {amount} of {cargo} to the Mining Vrata who take it for processing.]]), {
         amount   = NEEDED,
         cargo    = present.c,
      })
   end )

   vn.scene()
   local steve = vn.newCharacter( trader.vn_vrata_steve() )
   vn.transition()

   vn.na(_([[You are waiting for your reward when suddenly a big figure burst out of the Mining Vrata HQ.]]))
   steve(fmt.f(_([["Ho ho! Hey, you must be {player}. My name's Steve, and I'm in charge of the Mining Vrata right now."
He beams you a smile.
"I try my best to meet all new members! Got to keep the place lively!"]]), {
      player = player.name(),
   }))
   vn.menu{
      {_([["Nice to meet you."]]), "01_nice"},
      {_([["Member?"]]), "01_member"},
   }

   vn.label("01_member")
   steve(_([["Wait, what? You weren't told this delivery was the initiation rite for the Mining Vrata? Well, what's done is done. You're now a member, but don't worry, there are no fees or anything. Rather, this opens up more opportunities for you with the Space Trader's Society!"]]))
   vn.jump("01_cont")

   vn.label("01_nice")
   steve(_([["With that, you're now a proper member of the Mining Vrata. Don't worry though, there are no fees or anything. Rather, this opens up more opportunities for you with the Space Trader's Society!"]]))
   vn.jump("01_cont")

   vn.label("01_cont")
   steve( function ()
      local msg = _([["Oh yeah, here, let me get you with a reward hooked up for the minerals you brought in. We take pride in rewarding our members."]])
      if present.rare then
         return fmt.f(_([[{msg}
They take a quick look at their notes.
"Wait, you brought in {cargo}? That's quite rare. Let me increase your reward."]]), {
         cargo    = present.c,
         msg      = msg,
      })
      end
      return msg
   end )
   vn.sfxMoney()
   vn.na( function()
      local reward = REWARD
      if present.rare then
         reward = reward*2
      end
      player.pay( reward )
      return fmt.reward(reward)
   end )

   steve(_([["As a member, you'll have access to mining requests from mission bulletins. They're a good way to make mining more lucrative, benefit society, and the Mining Vrata. Keep your eye out for such requests when you yearn to mine."]]))
   steve(_([["Also, at the Mining Vrata we are looking for new resources and types of asteroid. If you ever find anything like that while mining, make sure to bring some over to any Trader's Society location! Will to drill!"]]))
   vn.na(_([[With the Mining Vrata's 'Will to Drill' motto echoing throughout the Mining Vrata HQ, Steve turns heels and stomps away.]]))

   vn.run()

   -- Player didn't hand anything in
   if not present then return end

   misn.finish(true)
end
