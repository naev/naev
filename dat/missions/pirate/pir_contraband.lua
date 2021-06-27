--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Pirate Smuggling">
  <avail>
   <priority>4</priority>
   <cond>faction.playerStanding("Pirate") &gt;= -100</cond>
   <chance>960</chance>
   <location>Computer</location>
   <done>Pirate Smuggle Cake</done>
  </avail>
  <notes>
   <tier>1</tier>
  </notes>
 </mission>
 --]]
--[[

   Handles the randomly generated Pirate contraband missions. They can appear
   anywhere and give better rewards with higher risk.

]]--

require "cargo_common"
require "numstring"


misn_desc = _("Smuggling contraband goods to %s in the %s system. Note that the cargo is illegal in most systems and you will face consequences if caught by patrols.")

msg_timeup = _("MISSION FAILED: You have failed to deliver the goods on time!")

osd_title = _("Smuggling %s")
osd_msg1 = _("Fly to %s in the %s system before %s\n(%s remaining)")

-- Use hidden jumps
cargo_use_hidden = false

-- Always available
cargo_always_available = true

--[[
--   Pirates shipping missions are always timed, but quite lax on the schedules
--   and pays a lot more then the rush missions
--]]

-- This is in cargo_common, but we need to increase the range
function cargo_selectMissionDistance ()
   return rnd.rnd( 5, 10 )
end


function create()
   -- Note: this mission does not make any system claims.
   
   -- Lower chance of appearing to 1/3 on non-pirate planets
   if planet.cur():faction() ~= faction.get("Pirate") and rnd.rnd() < 2/3 then
      misn.finish(false)
   end

   origin_p, origin_s = planet.cur()
   local routesys = origin_s
   local routepos = origin_p:pos()

   -- target destination
   destplanet, destsys, numjumps, traveldist, cargo, avgrisk, tier = cargo_calculateRoute()
   if destplanet == nil or destplanet:faction() == faction.get("Pirate") then
      misn.finish(false)
   end

   -- We’re redefining the cargo
   local cargoes = {
      -- Serious stuff here
      {N_("Stolen Goods"), N_("A lot of goods obtained by illicit and illegal means.")},
      {N_("Hacked Electronics"), N_("An assortment of illegally modified electronic goods.")},
      {N_("Illegal Waste"), N_("A diversity of highly illegal waste.")},
      {N_("Powerful Stimulants"), N_("A bunch of illegal stimulants.")},
      {N_("Unmarked Boxes"), N_("A bunch of unmarked boxes containing what you can only assume to be highly illegal items.")},
      {N_("Exotic Animals"), N_("A bunch of exotic animals that can not be legally traded.")},
      {N_("Radioactive Materials"), N_("Highly dangerous and illegal radioactive materials.")},
      {N_("Illegal Drugs"), N_("A bunch of drugs made illegal in most systems.")},
      {N_("Illegal Body Mods"), N_("A lot of illegal body modifications.")},
      {N_("Unauthorized Weapons"), N_("A bunch of illegal weapons.")},
      {N_("Contraband"), N_("An diverse assortment of illegal contraband goods.")},
      {N_("Counterfeit Goods"), N_("An assortment of illegal counterfeit goods of many famous brands.")},
      {N_("Stolen Art"), N_("Artwork that was illegally seized from a collector or gallery.")},
      -- Below less serious stuff
      {N_("User Databases"), N_("Stolen user databases with lots of personal information obtained illegally.")},
      {N_("Smelly Fruits"), N_("Illegal fruits that have a strong smell that can stink up entire stations in minutes.")},
      {N_("Catnip"), N_("Highly illegal drug that is very attractive to cats.")},
      {N_("Hypnotoads"), N_("Illegal amphibian with some mind-control abilities.")},
      {N_("Extra Spicy Burritos"), N_("Burritos that are so spicy, they are illegal.")},
      {N_("Pineapple Pizza"), N_("Pizza with pineapple on it, there is no way this is legal.")},
      {N_("Illicit Shader Code"), N_("Program used for rendering images with nefarious and illegal purposes.")},
   }
   local fact_cargoes = {
      ["Empire"] = {
         {N_("Tax-evasion Documents"), N_("Illegal documents detailing tax evasion by high empire officials.")},
         {N_("Leaked Documents"), N_("Documents illegally leaked from governmental entities.")},
         {N_("Incriminating Evidence"), N_("Illegally obtained evidence that incriminates high empire officials.")},
         {N_("Unauthorized Office Supplies"), N_("Office supplies that don't meet the standards of the Empire bureaucracy, including flimsy red staplers.")},
         {N_("Paper Deshredders"), N_("Devices that are able to reconstruct original documents from their shredded remains.")},
      },
      ["Dvaered"] = {
         {N_("Self-help Books"), N_("Books for self-betterment made illegal by the Dvaered authorities.")},
         {N_("Pacifist Manifestos"), N_("Documents making the case for pacifism, made illegal by the Dvaered authorities.")},
         {N_("Zen Rock Gardens"), N_("Sand and rock gardens with special rakes you can use for meditative purposes. Made illegal by the Dvaered authorities.")},
         {N_("Scented Soap"), N_("Fragrant soaps that can lead you to have a peace of mind. Made illegal by the Dvaered authorities.")},
      },
      ["Soromid"] = {
         {N_("Unstable DNA"), N_("Illegal DNA with strong reactive properties.")},
         {N_("Bio-weapons"), N_("Highly dangerous illegal biological weapons.")},
         {N_("Abducted Drosophila"), N_("Genetically modified small fruit flies illegally stolen from a laboratory.")},
         {N_("Mislabeled Plasmids"), N_("Independently replicating DNA that has been mislabeled and is no longer clear what exactly it does. Illegal for its uncertain and dangerous nature.")},
      },
      ["Sirius"] = {
         {N_("Heretical Documents"), N_("Illegal documents referring to heresy.")},
         {N_("Itch Powder"), N_("A banned substance with a history of disrupting robed processions.")},
         {N_("Unauthorized Sirichana Merchandise"), N_("Horribly tacky and outright bad merchandise with Sirichana pasted all over it. Sirichana toenail clipper anyone?")},
      },
      ["Za'lek"] = {
         {N_("Scientific Preprints"), N_("Non-paywalled illegal scientific papers.")},
         {N_("Sentient AI"), N_("Highly illegal AI that is assumed to be sentient.")},
         {N_("Bogus Proofs"), N_("Mathematical proofs with almost imperceptible modifications to make them illegally false.")},
         {N_("p-Hacked Results"), N_("Scientific trial results obtained through unreliable and illegal uses of statistics.")},
      }
   }
   -- Add faction cargoes as necessary
   fc = fact_cargoes[ destplanet:faction():nameRaw() ]
   if fc then
      for k,v in ipairs(fc) do
         table.insert( cargoes, v )
      end
   end
   -- Choose a random cargo and create it
   cargo = cargoes[rnd.rnd(1, #cargoes)]
   local c = misn.cargoNew( cargo[1], cargo[2] )
   -- TODO make this more nuanced
   c:illegalto( {"Empire", "Dvaered", "Soromid", "Sirius", "Za'lek"} )
   cargo = cargo[1] -- set it to name only

   -- mission generics
   stuperpx   = 0.3 - 0.015 * tier
   stuperjump = 11000 - 200 * tier
   stupertakeoff = 12000 - 50 * tier
   timelimit  = time.get() + time.create(0, 0, traveldist * stuperpx + numjumps * stuperjump + stupertakeoff + 240 * numjumps)

   -- Allow extra time for refuelling stops.
   local jumpsperstop = 3 + math.min(tier, 1)
   if numjumps > jumpsperstop then
      timelimit:add(time.create( 0, 0, math.floor((numjumps-1) / jumpsperstop) * stuperjump ))
   end
   
   -- Choose amount of cargo and mission reward. This depends on the mission tier.
   finished_mod = 2.0 -- Modifier that should tend towards 1.0 as Naev is finished as a game
   amount    = rnd.rnd(10 + 3 * tier, 20 + 4 * tier) 
   jumpreward = 2000
   distreward = 0.40
   reward    = 1.5^tier * (numjumps * jumpreward + traveldist * distreward) * finished_mod * (1. + 0.05*rnd.twosigma())
   
   misn.setTitle( string.format(
      _("PIRACY: Smuggle %s of %s"), tonnestring(amount),
      _(cargo) ) )
   misn.markerAdd(destsys, "computer")
   cargo_setDesc( misn_desc:format( destplanet:name(), destsys:name() ), cargo, amount, destplanet, timelimit );
   misn.setReward( creditstring(reward) )
end

-- Mission is accepted
function accept()
   local playerbest = cargoGetTransit( timelimit, numjumps, traveldist )
   if timelimit < playerbest then
      if not tk.yesno( _("Too slow"), string.format(
            _("This shipment must arrive within %s, but it will take at least %s for your ship to reach %s, missing the deadline. Accept the mission anyway?"),
            (timelimit - time.get()):str(), (playerbest - time.get()):str(),
            destplanet:name() ) ) then
         misn.finish()
      end
   end
   if player.pilot():cargoFree() < amount then
      tk.msg( _("No room in ship"), string.format(
         _("You don't have enough cargo space to accept this mission. It requires %s of free space (%s more than you have)."),
         tonnestring(amount),
         tonnestring( amount - player.pilot():cargoFree() ) ) )
      misn.finish()
   end

   misn.accept()

   carg_id = misn.cargoAdd( cargo, amount )
   tk.msg( _("Mission Accepted"), string.format(
      _("%s of %s are loaded onto your ship."), tonnestring(amount),
      _(cargo) ) )
   local osd_msg = {}
   osd_msg[1] = osd_msg1:format(
      destplanet:name(), destsys:name(), timelimit:str(),
      (timelimit - time.get()):str() )
   misn.osdCreate( string.format(osd_title,cargo), osd_msg)
   hook.land( "land" ) -- only hook after accepting
   hook.date(time.create(0, 0, 100), "tick") -- 100STU per tick
end

-- Land hook
function land()
   if planet.cur() == destplanet then
         tk.msg( _("Successful Delivery"), string.format(
            _("The containers of %s are unloaded at the docks."), _(cargo) ) )
      player.pay(reward)
      n = var.peek("ps_misn") or 0
      var.push("ps_misn", n+1)

      -- increase faction
      faction.modPlayerSingle("Pirate", rnd.rnd(2, 4))
      misn.finish(true)
   end
end

-- Date hook
function tick()
   if timelimit >= time.get() then
      -- Case still in time
      local osd_msg = {}
      osd_msg[1] = osd_msg1:format(
         destplanet:name(), destsys:name(), timelimit:str(),
         ( timelimit - time.get() ):str() )
      misn.osdCreate(string.format(osd_title,cargo), osd_msg)
   elseif timelimit <= time.get() then
      -- Case missed deadline
      player.msg(msg_timeup)
      misn.finish(false)
   end
end
