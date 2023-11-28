--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Pirate Convoy Raid">
 <priority>4</priority>
 <cond>faction.playerStanding("Pirate") &gt;= -20</cond>
 <chance>460</chance>
 <location>Computer</location>
 <faction>Wild Ones</faction>
 <faction>Black Lotus</faction>
 <faction>Raven Clan</faction>
 <faction>Dreamer Clan</faction>
 <faction>Pirate</faction>
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
local pir = require "common.pirate"
local fmt = require "format"
local flt = require "fleet"
local lmisn = require "lmisn"
local vntk = require "vntk"

local sconvoy, sescorts -- Non-persistent state

local function get_route( sys )
   local adj = sys:jumps()
   if #adj < 2 then return end
   local jumpenter, jumpexit
   local dist = 0
   for i,j1 in ipairs(adj) do
      for j,j2 in ipairs(adj) do
         if i~=j and not j1:exitonly() and not j1:hidden() and not j2:hidden() then
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
      -- Must not be claimed
      if not naev.claimTest( sys, true ) then
         return false
      end
      -- Must have two jumps that are fair away-ish
      local j1, j2, d = get_route( sys )
      if j1 == nil or j2 == nil or d < 10e3*10e3 then
         return false
      end

      return true
   end
   local syslist = lmisn.getSysAtDistance( nil, 2, 7, sysfilter, true )
   if #syslist <= 0 then
      misn.finish(false)
   end

   -- Choose system
   mem.targetsys = syslist[ rnd.rnd(1,#syslist) ]
   if not misn.claim( mem.targetsys, true ) then
      misn.finish(false)
   end

   local cargoes = {
      -- Standard stuff
      {N_("Corporate Documents"), N_("Documents detailing transactions and operations of certain corporations.")},
      {N_("Technology Blueprints"), N_("Blueprints of advanced technology under development.")},
      {N_("Research Prototypes"), N_("Advanced prototypes of cutting edge research. Doesn't seem to be of much use outside of an academic environment.")},
      {N_("High-end Implants"), N_("Some of the newest and fanciest cybernetic implants available. They include nose implants that allow amplifying and modifying smells beyond human imagination.")},
      {N_("Synthetic Organs"), N_("Special synthetic copies of natural human organs.")},
      {N_("Brand Goods"), N_("A variety of high quality brand luxury goods.")},
      {N_("Rare Ores"), N_("Rare ores that are usually only located in isolated asteroid fields.")},
      {N_("Fine Arts"), N_("Museum-quality artwork done in all sorts of mediums.")},
      {N_("Highly Refined Metals"), N_("High quality refined metals suitable for building space craft and other advanced technological objects.")},
      {N_("Nebula Artefacts"), N_("Rare and weird artefacts of ship debris and unidentified objects found in the Nebula.")},
      {N_("Ancient Code"), N_("Ancient code designed mainly for computers and operating systems that no longer exist, and of high value to software archaeologists.")},
      {N_("Premium Service Droids"), N_("Expensive service droids that can do all sorts of menial tasks.")},
      -- A bit sillier
      {N_("Aged Wines"), N_("Wines aged in carefully controlled climates to bring out the most subtle and exquisite flavours.")},
      {N_("Fine-tuned Neural Networks"), N_("Neural networks trained with billions of data points to do all sorts of pointless tasks.")},
      {N_("Blue and White Porcelain"), N_("Very exquisite and highly detailed traditional blue and white pottery.")},
      {N_("Exquisite Shaders"), N_("Code for exquisite shaders that are able to render computer graphics scenes beyond all human imagination.")},
      {N_("Audiophile Paraphernalia"), N_("High end audio systems meant for true audio connoisseurs.")},
      {N_("Rare Plants"), N_("High quality and rare specimens of plants.")},
      {N_("Ornamental Shrimp"), N_("An assortment of small and colourful shrimp.")},
      {N_("High Quality Pasta"), N_("Dried pasta of the highest quality.")},
      {N_("Premium Body Soap"), N_("Incredibly silky soap that creates a seemingly infinite amount of bubbles.")},
      {N_("Luxury Captain Chairs"), N_("Very comfortable chairs meant for ship captains. Every captain dreams of having such chairs.")},
      {N_("Incredibly Spicy Sauce"), N_("Hot sauce made from the spiciest, genetically engineered peppers. Not really suited for human consumption, but people use them anyway.")},
      {N_("Exquisite Cat Toys"), N_("Cat toys with built in lights and motion systems to stimulate any cat to the max. They also don't use cheap glue that make them break down within 5 minutes of playing with a cat.")},
   }

   -- Finish mission details
   mem.returnpnt, mem.returnsys = spob.cur()
   mem.cargo = cargoes[ rnd.rnd(1,#cargoes) ]
   mem.misn_cargo = commodity.new( mem.cargo[1], mem.cargo[2] )
   mem.enemyfaction = faction.get("Trader")
   mem.convoy_enter, mem.convoy_exit = get_route( mem.targetsys )
   -- TODO make tiers based on how many times the player does them or something
   local r = rnd.rnd()
   local done = var.peek("pir_convoy_raid") or 0
   local mod = math.exp( -done*0.05 ) -- 0.95 for 1, 0.90 for 2 0.86 for 3, etc.
   mod = math.max( 0.5, mod ) -- Limit it so that 50% are large
   if r < 0.5*mod then
      mem.tier = 1
      mem.adjective = p_("raid", "tiny")
   elseif r < 1.0*mod then
      mem.tier = 2
      mem.adjective = p_("raid", "small")
   elseif r < 1.2*mod then
      mem.tier = 3
      mem.adjective = p_("raid", "medium")
   else
      mem.tier = 4
      mem.adjective = p_("raid", "large")
   end

   -- Set up rewards
   mem.reward_faction = pir.systemClanP( system.cur() )
   mem.reward_base = 25e3 + rnd.rnd() * 15e3 + 25e3*math.sqrt(mem.tier)
   mem.reward_cargo = 2e3 + rnd.rnd() * 2e3 + 3e3*math.sqrt(mem.tier)

   local faction_text = pir.reputationMessage( mem.reward_faction )
   misn.setTitle(fmt.f(pir.prefix(mem.reward_faction).._("Raid a {adjective} {name} convoy in the {sys} system"), {adjective=mem.adjective, name=mem.enemyfaction, sys=mem.targetsys} ))
   misn.setDesc(fmt.f(_("A convoy carrying {cargo} will be passing through the {sys} system on the way from {entersys} to {exitsys}. A local Pirate Lord wants you to assault the convoy and bring back as many tonnes of {cargo} as possible. You will be paid based on how much you are able to bring back.{reputation}"),
         {cargo=mem.misn_cargo, sys=mem.targetsys, entersys=mem.convoy_enter:dest(), exitsys=mem.convoy_exit:dest(), reputation=faction_text}))
   misn.setReward(fmt.f(_("{rbase} and {rcargo} per ton of {cargo} recovered"), {rbase=fmt.credits(mem.reward_base), rcargo=fmt.credits(mem.reward_cargo), cargo=mem.misn_cargo}))
   misn.markerAdd( mem.targetsys )
end

function accept ()
   misn.accept()

   misn.osdCreate(_("Pirate Raid"), {
      fmt.f(_("Go to the {sys} system"),{sys=mem.targetsys}),
      fmt.f(_("Plunder {cargo} from the convoy"),{cargo=mem.misn_cargo}),
      fmt.f(_("Deliver the loot to {pnt} in the {sys} system"),{pnt=mem.returnpnt, sys=mem.returnsys}),
   } )

   hook.enter("enter")
   hook.land("land")
end

function enter ()
   local q = player.fleetCargoOwned( mem.misn_cargo )
   if mem.convoy_spawned and q <= 0 then
      lmisn.fail(fmt.f(_("You did not recover any {cargo} from the convoy!"), {cargo=mem.misn_cargo}))
   end
   if mem.convoy_spawned and q > 0 then
      misn.osdActive(3)
   end
   if system.cur() ~= mem.targetsys or mem.convoy_spawned then
      return
   end

   mem.convoy_spawned = true
   misn.osdActive(2)
   hook.timer( 10+5*rnd.rnd(), "enter_delay" )
end

function land ()
   local q = player.fleetCargoOwned( mem.misn_cargo )
   if mem.convoy_spawned and q > 0 and spob.cur()==mem.returnpnt then
      q = player.fleetCargoRm( mem.misn_cargo, q ) -- Remove it
      local reward = mem.reward_base + q * mem.reward_cargo
      lmisn.sfxVictory()
      vntk.msg( _("Mission Success"), fmt.f(_("The workers unload your {cargo} and take it away to somewhere you can't see. As you wonder about your payment, you suddenly receive a message that #g{reward}#0 was transferred to your account."), {cargo=mem.misn_cargo, reward=fmt.credits(reward)}) )
      player.pay( reward )

      -- Faction hit
      faction.modPlayerSingle(mem.reward_faction, mem.tier*(rnd.rnd(1, 2)+math.min(q*3/100,3)))

      -- Mark as done
      local done = var.peek( "pir_convoy_raid" ) or 0
      var.push( "pir_convoy_raid", done+1 )

      pir.addMiscLog(fmt.f(_("You raided a {adjective} {name} convoy in the {sys} system and stole {tonnes} of {cargo}."), {adjective=mem.adjective, name=mem.enemyfaction, sys=mem.targetsys, tonnes=fmt.tonnes(q), cargo=mem.misn_cargo}))
      misn.finish(true)
   end
end

function enter_delay ()
   mem.mrkentry = system.markerAdd( mem.convoy_enter:pos(), _("Convoy Entry Point") )
   mem.mrkexit = system.markerAdd( mem.convoy_exit:pos(), _("Convoy Exit Point") )

   player.autonavReset( 5 )
   player.msg(fmt.f(_("The convoy will be coming in from {sys} shortly!"), {sys=mem.convoy_enter:dest()}))
   hook.timer( 5+10*rnd.rnd(), "spawn_convoy" )
end

function spawn_convoy ()
   -- Tier 1
   local tships, eships
   if mem.tier==4 then
      tships = {"Mule", "Mule", "Mule"}
      if rnd.rnd() < 0.5 then
         table.insert( tships, "Mule" )
      end
      if rnd.rnd() < 0.5 then
         eships = {"Kestrel"}
      else
         eships = {"Pacifier", "Pacifier"}
      end
      for i=1,rnd.rnd(4.5) do
         table.insert( eships, (rnd.rnd() < 0.7 and "Lancelot") or "Admonisher" )
      end

   elseif mem.tier==3 then
      tships = {"Mule"}
      local r = rnd.rnd()
      if r < 0.3 then
         eships = {"Pacifier"}
      elseif  r < 0.6 then
         eships = {"Vigilance"}
      else
         eships = {"Admonisher", "Admonisher"}
      end
      for _i=1,rnd.rnd(3.4) do
         table.insert( eships, (rnd.rnd() < 0.7 and "Shark") or "Lancelot" )
      end

   elseif mem.tier==2 then
      if rnd.rnd() < 0.5 then
         tships = {"Koala", "Koala"}
      else
         tships = {"Koala", "Llama", "Llama"}
      end
      if rnd.rnd() < 0.5 then
         eships = { "Lancelot" }
      else
         eships = { "Vendetta" }
      end
      for _i=1,3 do
         table.insert( eships, "Shark" )
      end

   else -- mem.tier==1
      tships = {"Llama", "Llama"}
      eships = {"Shark", "Shark"}
      if rnd.rnd() < 0.5 then
         table.insert( eships, "Shark" )
      end
   end

   -- Use a dynamic faction so pirates don't destroy them
   local fconvoy = faction.dynAdd( mem.enemyfaction, "convoy_faction", mem.enemyfaction:name(), {clear_enemies=true, clear_allies=true} )

   sconvoy = flt.add( 1, tships, fconvoy, mem.convoy_enter, _("Convoy") )
   for _k,p in ipairs(sconvoy) do
      p:cargoRm("all")
      p:cargoAdd( mem.misn_cargo, math.floor((0.8+0.2*rnd.rnd())*p:cargoFree()) )
      hook.pilot( p, "board", "convoy_board" )
      hook.pilot( p, "attacked", "convoy_attacked" )
   end
   sescorts = flt.add( 1, eships, fconvoy, mem.convoy_enter, nil, {ai="mercenary"} )
   for _k,p in ipairs(sescorts) do
      p:setLeader( sconvoy[1] )
      hook.pilot( p, "attacked", "convoy_attacked" )
   end
   -- Only slow down leader, or it can be faster than other guys
   sconvoy[1]:intrinsicSet( "speed_mod", -33 )
   sconvoy[1]:intrinsicSet( "accel_mod", -33 )
   sconvoy[1]:intrinsicSet( "turn_mod", -33 )
   sconvoy[1]:setHilight(true)
   sconvoy[1]:control()
   sconvoy[1]:hyperspace( mem.convoy_exit )
   hook.pilot( sconvoy[1], "death", "convoy_done" )
end

function convoy_attacked ()
   for _k,p in ipairs(sconvoy) do
      if p:exists() then
         p:setHostile(true)
      end
   end
   for _k,p in ipairs(sescorts) do
      if p:exists() then
         p:setHostile(true)
      end
   end
end

function convoy_board ()
   hook.timer( 1, "convoy_boarded" )
   convoy_done()
end

function convoy_boarded ()
   if player.fleetCargoOwned( mem.misn_cargo ) > 0 then
      misn.osdGetActive(3)
   end
end

function convoy_done ()
   system.markerClear()
end
