--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Join Astra Vigilis">
 <unique/>
 <priority>4</priority>
 <chance>50</chance>
 <location>Bar</location>
 <cond>
   if spob.cur():faction() ~= faction.get("Traders Society") and rnd.rnd() > 0.1 then
      return false
   end
   local misn_test = require("misn_test")
   return misn_test.generic() and misn_test.reweight_active()
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
local pir = require "common.pirate"
local lmisn = require "lmisn"
local pilotname = require "pilotname"
local bounty = require "common.bounty"

local TITLE    = _("Join the Astra Vigilis")
local FACTION  = faction.get("Traders Society")
local SHIP     = ship.get("Pirate Hyena")
local CREDITS  = 100e3
local REWARD   = outfit.get("Mercenary Licence")

local NPCNAME  = _("Astra Vigilis Recruiter")
local npcvn, npcpor = vni.generic()
function create ()
   local systems = lmisn.getSysAtDistance( system.cur(), 1, 3,
      function(s)
         -- Must be claimable
         if not naev.claimTest( s, true ) then
            return false
         end
         local p = pir.systemPresence( s )
         return p > 0 and p < 200
      end )
   if #systems <= 0 then return misn.finish(false) end
   mem.sys = systems[ rnd.rnd( 1, #systems ) ]
   if not misn.claim( mem.sys, true ) then return end

   mem.name = pilotname.pirate()

   misn.setNPC( NPCNAME, npcpor, _("You see a recruiter for the Astra Vigilis mercenary guild.") )
   misn.setTitle( TITLE )
   misn.setReward( fmt.f(_([[{license} and {credits}]]), {
      license = tostring(REWARD),
      credits = fmt.credits(CREDITS),
   }))
   misn.setDesc(_([[You have been given a bounty to complete to join the Astra Vigilis mercenary guild.]]))
end

function accept ()
   local accepted

   vn.reset()
   vn.scene()
   local npc = vn.newCharacter( NPCNAME, { image=npcvn } )
   vn.transition()

   vn.na(_([[You approach the Astra Vigilis recruiter who has clearly left a seat in front of them for you to sit down."]]))
   npc(_([["Why hello there pilot! You interested in carving a name in history for yourself with the Astra Vigilis?"]]))
   vn.menu{
      { _([["Astra Vigilis?"]]), "01_astra_vigilis" },
      { _("Ask for details."), "01_details" },
      { _("Maybe later."), "01_later" },
   }

   vn.label("01_astra_vigilis")
   npc(_([["You must have heard of us. The Astra Vigilis is the largest mercenary guild of the Space Trader's Society. We play an important role in keeping peace and order. Our mere existence is enough to deter piracy and law-breaking from happening in the first place!"]]))
   npc(_([[They lean forward with a meticulously calculated gleam of excitement in their eyes.
"Not only is the pay great, but as you get a better reputation for a job well done you'll get access to a lot of fancy new perks. However, the adrenaline rush from bounty hunting itself is the largest reward of all!"]]))
   vn.menu{
      { _([["If the Astra Vigilis is so good, why so many pirates then?"]]), "01_pirates" },
      { _("Ask for details."), "01_details" },
      { _("Maybe later."), "01_later" },
   }

   vn.label("01_pirates")
   npc(_([[Their face briefly seems to contort into an expression of frustration before the recruiter training kicks into overdrive, and their face melds once again into a friendly expression.
"That's what we're looking to fix with a new hiring campaign, and you look like exactly the type of pilot that can fix it!"]]))
   vn.menu{
      { _("Ask for details."), "01_details" },
      { _("Maybe later."), "01_later" },
   }

   vn.label("01_details")
   npc(_([[Their eyes bright in a way only recruiters can do.
"Great! Now, joining the Astra Vigilis isn't as simple as just doing paperwork or buying a mercenary license. You have to prove that you have the prowess to take down bounties."]]))
   npc(fmt.f(_([["It seems like there's just the thing for you. There's a pirate by the name of {name} who is usually raiding around the {sys} system, and is flying a {ship}. Cleaning them out would make you eligible for joining the Astra Vigilis and partaking in much more challenging requests in the future. What do you say? Are you ready to go guns blazing?"]]), {
      name  = mem.name,
      sys   = mem.sys,
      ship  = SHIP,
   }))
   vn.menu{
      { _("Accept."), "01_accept" },
      { _("Maybe later."), "01_later" },
   }

   vn.label("01_accept")
   vn.func( function ()
      accepted = true
   end )
   npc(fmt.f(_([["Great! I've sent you the data. After you have dealt with {name}, please report to any {fct} planet or station to finish the paperwork."]]), {
      name  = mem.name,
      fct   = FACTION,
   }))

   vn.label("01_later")
   vn.run()

   if not accepted then return end

   bounty.init( mem.sys, mem.name, SHIP, REWARD, {
      payingfaction  = FACTION,
      reputation     = 20,
      targetfaction  = faction.get("Marauder"),
      alive_only     = false,
      spawnfunc      = "spawn_pirate",
      completefunc   = "land_done",
      osd_title      = TITLE,
      osd_reward     = fmt.f(_("Land at a {fct} station or planet"),{fct=FACTION}),
   } )
   bounty.accept()
end

-- luacheck: globals spawn_pirate
function spawn_pirate( b )
   -- As easy as possible
   local p = pilot.add( b.targetship[1], bounty.get_faction(), bounty.choose_spawn_pos(), b.targetname, {
      ai          = "baddie",
      -- If we make it worn down, the player feels a big jump in difficulty with the "real bounties"
      --intrinsics  = { outfit.get("Worn Down") },
   } )
   local aimem = p:memory()
   aimem.loiter = math.huge -- Should make them loiter forever
   aimem.capturable = true
   p:setVisplayer(true)
   return p
end

-- luacheck: globals land_done
function land_done ()
   vn.reset()
   vn.scene()
   vn.transition()

   vn.na(fmt.f(_([[You land on {spb}, and make your way to register the bounty as completed, and fill out the necessary paperwork to join the Astra Vigilis. After being somewhat used to Imperial paperwork, you find the process to be much more simple and straight forward. Bounty hunters don't like to put up with too much bullshit, and you are grateful for that.]]), {
      spb = spob.cur(),
   }))
   vn.na(fmt.f(_([[With the paperwork over, you are promptly handed a {reward} that marks you as a full fledged member of the Astra Vigilis. You will now have access to bounty missions, and the Astra Vigilis terminal on {faction} stations and planets that can customize your bounty hunting experience.]]), {
      faction  = FACTION,
      reward   = REWARD,
   }))

   -- Start with easy bounties
   var.push( "bounty_difficulty", N_("Easy") )

   vn.func( function ()
      FACTION:hit(25)
      player.outfitAdd( REWARD )
   end )
   vn.sfxVictory()
   vn.na( fmt.reward(CREDITS).."\n"..fmt.reward(REWARD) )

   vn.run()

   trader.addAstraVigilisLog(fmt.f(_([[You passed a bounty hunting test by defeating the notorious pirate {pirname} given to you by the Astra Vigilis, and obtained a {reward}. As a new member of the Astra Vigilis, you can now access bounty missions.]]), {
      reward   = REWARD,
      pirname  = mem.name,
   } ))
   misn.finish(true)
end
