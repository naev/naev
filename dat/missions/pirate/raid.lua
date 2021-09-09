--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Pirate Convoy Raid">
 <avail>
  <priority>4</priority>
  <cond>faction.playerStanding("Pirate") &gt;= -20</cond>
  <chance>460</chance>
  <location>Computer</location>
  <faction>Wild Ones</faction>
  <faction>Black Lotus</faction>
  <faction>Raven Clan</faction>
  <faction>Dreamer Clan</faction>
  <faction>Pirate</faction>
 </avail>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[
   Have to raid a convoy and bring stuff back.
   1. Convoy moves slower because material needs careful delivery.
   2. Have to disable convoy ships and recover stuff.
   3. Payment is based on how much stuff is recovered.
--]]
local pir = require "missions.pirate.common"
local fmt = require "format"
local flt = require "fleet"
local lmisn = require "lmisn"
local vntk = require "vntk"
require "jumpdist"

function get_route( sys )
   local adj = sys:jumps()
   if #adj < 2 then return end
   local jumpenter, jumpexit
   local dist = 0
   for i,j1 in ipairs(adj) do
      for j,j2 in ipairs(adj) do
         if j1 ~= j2 then
            local d = j1:pos():dist2(j2:pos())
            if d > dist then
               dist = d
               jumpenter = j1
               jumpexit = j2
            end
         end
      end
   end
   if rnd.rnd() < 0.5 then
      return jumpenter, jumpexit, dist
   else
      return jumpexit, jumpenter, dist
   end
end

function create ()
   if not var.peek('testing') then misn.finish(false) end
   local target_factions = {
      "Independent",
      "Trader",
      "Empire",
      "Soromid",
      "Sirius",
      "Dvaered",
      "Za'lek",
   }
   -- Choose system and try to claim
   local sysfilter = function ( sys )
      local p = sys:presences()
      local total =  0
      for k,v in ipairs(target_factions) do
         total = total + (p[v] or 0)
      end
      local pirates = pir.systemPresence(sys)
      -- Some presence check
      if total < 300 or pirates > total then
         return false
      end
      -- Must not be cliamed
      if not misn.claim( sys, true ) then
         return false
      end
      -- Must have two jumps
      local j1, j2, d = get_route( sys )
      if j1 ~= nil and d < 10e3*10e3 then
         return false
      end

      return true
   end
   local syslist = getsysatdistance( nil, 2, 7, sysfilter, true )
   if #syslist <= 0 then
      misn.finish(false)
   end

   -- Choose system
   targetsys = syslist[ rnd.rnd(1,#syslist) ]
   if not misn.claim( targetsys ) then
      misn.finish(false)
   end

   local cargoes = {
      -- Standard stuff
      {N_("Corporate Documents"), N_("Documents detailing transactions and operations of certain corporations.")},
      {N_("Technology Blueprints"), N_("Blueprints of under development advanced technology.")},
      {N_("Research Prototypes"), N_("Advanced prototypes of cutting edge research. Doesn't seem like of much use outside of an academic environment.")},
      {N_("High-end Implants"), N_("Some of the newest and fanciest cybernetic implants available. They included nose implants that allow amplifying and modifying smells beyond human imagination.")},
      {N_("Synthetic Organs"), N_("Special synthetic copies of natural human organs that are able to ")},
      -- A bit sillier
      {N_("Audiophile Paraphernalia"), N_("High end audio systems meant for true audio connoisseurs.")},
      {N_("Rare Plants"), N_("High quality and rare specimens of plants.")},
      {N_("Ornamental Shrimp"), N_("An assortment of small and colourful shrimp.")},
      {N_("High Quality Pasta"), N_("Dried pasta of the highest quality.")},
      {N_("Premium Body Soap"), N_("Incredibly silky soap that creates a seemingly infinite amount of bubbles.")},
      {N_("Luxury Captain Chairs"), N_("Very comfortable chairs meant for ship captains. Every captain dreams of having such chairs.")},
      {N_("Incredibly Spicy Sauce"), N_("Hot sauce made from the spiciest peppers that have been genetically engineered. Not really suited for human consumption, but people use them anyway.")},
      {N_("Exquisite Cat Toys"), N_("Cat toys with built in light and motion system to stimulate any cat to the max. They also don't use cheap glue that make them break down within 5 minutes of playing with a cat.")},
   }

   -- Finish mission details
   returnpnt, returnsys = planet.cur()
   cargo = cargoes[ rnd.rnd(1,#cargoes) ]
   cargo.__save = true
   misn_cargo = misn.cargoNew( cargo[1], cargo[2] )
   enemyfaction = faction.get("Trader")
   convoy_enter, convoy_exit = get_route( targetsys )
   -- TODO make tiers based on how many times the player does them or something
   local r = rnd.rnd()
   if r < 0.5 then
      tier = 1
   elseif r < 0.8 then
      tier = 2
   else
      tier = 3
   end

   -- Set up rewards
   reward_faction = pir.systemClanP( system.cur() )
   reward_base = 100e3 + rnd.rnd() * 50e3
   reward_cargo = 10e3 + rnd.rnd() * 3e3

   local faction_text = pir.reputationMessage( reward_faction )
   local faction_title = ""
   if pir.factionIsClan( reward_faction ) then
      faction_title = fmt.f(_(" ({clanname})"), {clanname=reward_faction:name()})
   end

   misn.setTitle(fmt.f(_("#rPIRACY#0: Raid a {name} convoy in the {sys} system{fct}"), {name=enemyfaction:name(), sys=targetsys:name(), fct=faction_title} ))
   misn.setDesc(fmt.f(_("A convoy carrying {cargo} will be passing through the {sys} system on the way from {entersys} to {exitsys}. A local Pirate Lord wants you to assault the convoy and bring back as much {cargo} as possible. You will be paid based on how much you are able to bring back.{reputation}"),
         {cargo=cargo[1], sys=targetsys:name(), convoy_enter:dest():name(), convoy_exit:dest():name(), reputation=faction_text}))
   misn.setReward(fmt.f(_("{rbase} and {rcargo} per ton of {cargo} recovered"), {rbase=fmt.credits(reward_base),rcargo=fmt.credits(reward_cargo),cargo=cargo[1]}))
   misn.markerAdd( targetsys )
end

function accept ()
   misn.accept()

   misn.osdCreate(_("Pirate Raid"), {
      fmt.f(_("Go to the {sysname} system"),{sysname=targetsys:name()}),
      fmt.f(_("Plunder {cargoname} from the convoy"),{cargoname=cargo[1]}),
      fmt.f(_("Deliver the loot to {pntname} in the {sysname} system"),{pntname=returnpnt:name(), sysname=returnsys:name()}),
   } )

   hook.enter("enter")
   hook.land("land")
end

function enter ()
   if convoy_spawned and player.pilot():cargoHas( misn_cargo ) <= 0 then
      player.msg(fmt.f(_("#rMISSION FAILED: You did not recover any {cargoname} from the convoy!"), {cargoname=cargo[1]}))
      misn.finish(false)
   end
   if system.cur() ~= targetsys or convoy_spawned then
      return
   end

   convoy_spawned = true
   misn.osdActive(2)
   hook.timer( 10+5*rnd.rnd(), "enter_delay" )
end

function land ()
   local pp = player.pilot()
   local q = pp:cargoHas( misn_cargo )
   if convoy_spawned and q > 0 then
      local reward = reward_base + q * reward_cargo
      lmisn.sfxVictory()
      vntk.msg( nil, fmt.f(_("The workers unload your {cargoname} and take it away to somewhere you can't see. As you wonder about your payment, you suddenly receive a message that #g{reward}#0 was transferred to your account."), {cargoname=cargo[1], reward=fmt.credits(reward)}) )
      player.pay( reward )
      pp:cargoRm( misn_cargo, q )
      misn.finish(true)
   end
end

function enter_delay ()
   mrkentry = system.mrkAdd( _("Convoy Entry Point"), convoy_enter:pos() )
   mrkexit = system.mrkAdd( _("Convoy Exit Point"), convoy_exit:pos() )

   player.autonavReset( 5 )
   player.msg(fmt.f(_("The convoy will be coming in from {sysname} shortly!"), {sysname=convoy_enter:name()}))
   hook.timer( 5+10*rnd.rnd(), "spawn_convoy" )
end

function spawn_convoy ()
   -- Tier 1
   local tships = {"Koala", "Llama", "Llama"}
   local eships
   if rnd.rnd() < 0.5 then
      eships = {"Shark", "Shark"}
   else
      eships = {"Shark", "Hyena", "Hyena"}
   end
   sconvoy = flt.add( 1, tships, enemyfaction, convoy_enter, _("Convoy") )
   for k,p in ipairs(sconvoy) do
      p:cargoRm("all")
      p:cargoAdd( misn_cargo, math.floor((0.8+0.2*rnd.rnd())*p:cargoFree()) )
      p:intrinsicSet( "speed_mod", -33 )
      p:intrinsicSet( "thrust_mod", -33 )
      p:intrinsicSet( "turn_mod", -33 )
      hook.pilot( p, "board", "convoy_board" )
   end
   sescorts = flt.add( 1, eships, enemyfaction, convoy_enter )
   for k,p in ipairs(sescorts) do
      p:setLeader( sconvoy[1] )
   end
   sconvoy[1]:setHilight(true)
   sconvoy[1]:control()
   sconvoy[1]:hyperspace( convoy_exit, true )
   hook.pilot( sconvoy[1], "death", "convoy_done" )
end

function convoy_board ()
   hook.timer( 1, "convoy_boarded" )
   convoy_done()
end

function convoy_boarded ()
   if player.pilot():cargoHas( misn_cargo ) > 0 then
      misn.osdGetActive(3)
   end
end

function convoy_done ()
   system.mrkClear()
end
