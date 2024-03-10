--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Pirate Smuggling">
 <priority>4</priority>
 <cond>faction.playerStanding("Pirate") &gt;= -100</cond>
 <chance>960</chance>
 <location>Computer</location>
 <done>Pirate Smuggle Cake</done>
 <notes/>
</mission>
--]]
--[[

   Handles the randomly generated Pirate contraband missions. They can appear
   anywhere and give better rewards with higher risk.

]]--
local pir = require "common.pirate"
local car = require "common.cargo"
local fmt = require "format"
local lmisn = require "lmisn"
local vntk = require "vntk"

--[[
--   Pirates shipping missions are always timed, but quite lax on the schedules
--   and pays a lot more then the rush missions
--]]

-- This is in common.cargo, but we need to increase the range
function create()
   -- Note: this mission does not make any system claims.
   local pntf = spob.cur():faction()
   -- Lower chance of appearing to 1/3 on non-pirate planets
   if not pir.factionIsPirate( pntf ) and rnd.rnd() < 2/3 then
      misn.finish(false)
   end
   -- Doesn't appear on Thurion and Proteron worlds for now
   if pntf == faction.get("Thurion") or pntf == faction.get("Proteron") then
      misn.finish(false)
   end

   mem.reward_faction = pir.systemClanP( system.cur() )
   local faction_text = pir.reputationMessage( mem.reward_faction )

   mem.origin_p, mem.origin_s = spob.cur()

   -- target destination. Override "always_available" to true.
   mem.destplanet, mem.destsys, mem.numjumps, mem.traveldist, mem.cargo, mem.avgrisk, mem.tier = car.calculateRoute( rnd.rnd(5, 10), {always_available=true} )
   if mem.destplanet == nil or pir.factionIsPirate( mem.destplanet:faction() ) then
      misn.finish(false)
   end

   -- We’re redefining the cargo
   local cargoes = {
      -- Serious stuff here
      {N_("Stolen Goods"), N_("A lot of goods obtained by illicit and illegal means.")},
      {N_("Hacked Electronics"), N_("An assortment of illegally modified electronic goods.")},
      {N_("Illegal Waste"), N_("A melange of highly illegal waste.")},
      {N_("Powerful Stimulants"), N_("A bunch of illegal stimulants.")},
      {N_("Unmarked Boxes"), N_("A multitude of unmarked boxes containing what you can only assume to be highly illegal items.")},
      {N_("Exotic Animals"), N_("A collection of exotic animals that can not be legally traded.")},
      {N_("Radioactive Materials"), N_("Highly dangerous and illegal radioactive materials.")},
      {N_("Illegal Drugs"), N_("A batch of drugs made illegal in most systems.")},
      {N_("Illegal Body Mods"), N_("An array of illegal body modifications.")},
      {N_("Unauthorized Weapons"), N_("An assemblage of illegal weapons.")},
      {N_("Contraband"), N_("A diverse assortment of illegal contraband goods.")},
      {N_("Counterfeit Goods"), N_("An assortment of illegal counterfeit goods of many famous brands.")},
      {N_("Stolen Art"), N_("Artwork that was illegally seized from a collector or gallery.")},
      -- Below less serious stuff
      {N_("User Databases"), N_("Stolen user databases with lots of personal information obtained illegally.")},
      {N_("Smelly Fruits"), N_("Illegal fruits that have a strong smell that can stink up entire stations in minutes.")},
      {N_("Catnip"), N_("Highly illegal drug that is very attractive to cats.")},
      {N_("Hypnotoads"), N_("Illegal amphibian with some mind-control abilities.")},
      {N_("Extra Spicy Burritos"), N_("Burritos that are so spicy, they are illegal.")},
      {N_("Pineapple Pizza"), N_("Pizza with pineapple on it. There is no way this is legal.")},
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
         {N_("Bio-weapons"), N_("Highly dangerous, illegal biological weapons.")},
         {N_("Abducted Drosophila"), N_("Genetically modified small fruit flies illegally stolen from a laboratory.")},
         {N_("Mislabeled Plasmids"), N_("Independently replicating DNA that has been mislabeled and is no longer clear what exactly it does. Illegal for its uncertain and dangerous nature.")},
         {N_("Soromid Cheese"), N_("The 'don't ask, don't tell' of the cheese world. It is completely covered with puffy green and blue mold.")},
      },
      ["Sirius"] = {
         {N_("Heretical Documents"), N_("Illegal documents referring to heresy.")},
         {N_("Itch Powder"), N_("A banned substance with a history of disrupting robed processions.")},
         {N_("Unauthorized Sirichana Merchandise"), N_("Horribly tacky and outright bad merchandise with Sirichana pasted all over it. Sirichana toenail clipper anyone?")},
         {N_("Tinfoil Caps"), N_("Tinfoil caps that do not look very stylish.")},
      },
      ["Za'lek"] = {
         {N_("Scientific Preprints"), N_("Non-paywalled illegal scientific papers.")},
         {N_("Sentient AI"), N_("Highly illegal AI that is assumed to be sentient.")},
         {N_("Bogus Proofs"), N_("Mathematical proofs with almost imperceptible modifications to make them illegally false.")},
         {N_("p-Hacked Results"), N_("Scientific trial results obtained through unreliable and illegal uses of statistics.")},
      }
   }
   -- Add faction cargoes as necessary
   local fc = fact_cargoes[ mem.destplanet:faction():nameRaw() ]
   if fc then
      for k,v in ipairs(fc) do
         table.insert( cargoes, v )
      end
   end
   -- Choose a random cargo and create it
   mem.cargo = cargoes[rnd.rnd(1, #cargoes)]
   local c = commodity.new( mem.cargo[1], mem.cargo[2] )
   -- TODO make this more nuanced
   c:illegalto( {"Empire", "Dvaered", "Soromid", "Sirius", "Za'lek", "Frontier"} )
   mem.cargo = mem.cargo[1] -- set it to name only

   -- mission generics
   local stuperpx   = 0.3 - 0.015 * mem.tier
   local stuperjump = 11000 - 200 * mem.tier
   local stupertakeoff = 12000 - 50 * mem.tier
   mem.timelimit  = time.get() + time.new(0, 0, mem.traveldist * stuperpx + mem.numjumps * stuperjump + stupertakeoff + 240 * mem.numjumps)

   -- Allow extra time for refuelling stops.
   local jumpsperstop = 3 + math.min(mem.tier, 1)
   if mem.numjumps > jumpsperstop then
      mem.timelimit:add(time.new( 0, 0, math.floor((mem.numjumps-1) / jumpsperstop) * stuperjump ))
   end

   -- Choose amount of cargo and mission reward. This depends on the mission tier.
   mem.amount    = rnd.rnd(10 + 3 * mem.tier, 20 + 4 * mem.tier)
   local jumpreward = 3000
   local distreward = 0.50
   mem.reward    = 1.5^mem.tier * (mem.numjumps * jumpreward + mem.traveldist * distreward + math.max(1,mem.amount/20)) * (1 + 0.05*rnd.twosigma())

   misn.setTitle( fmt.f(
      pir.prefix(mem.reward_faction).._("Smuggle {tonnes} of {cargo}"),
         {tonnes=fmt.tonnes(mem.amount), cargo=_(mem.cargo)} ) )
   misn.markerAdd(mem.destplanet, "computer")
   if pir.factionIsPirate( spob.cur():faction() ) then
      car.setDesc( fmt.f( _("Smuggling contraband goods to {pnt} in the {sys} system.{msg}"), {pnt=mem.destplanet, sys=mem.destsys, msg=faction_text} ), mem.cargo, mem.amount, mem.destplanet, mem.timelimit )
   else
      car.setDesc( fmt.f( _("Smuggling contraband goods to {pnt} in the {sys} system.{msg}"), {pnt=mem.destplanet, sys=mem.destsys, msg=faction_text} ) .. "\n\n" .. _("#rWARNING:#0 Contraband is illegal in most systems and you will face consequences if caught by patrols."), mem.cargo, mem.amount, mem.destplanet, mem.timelimit )
   end

   misn.setReward(mem.reward)
end

-- Mission is accepted
function accept()
   local playerbest = car.getTransit( mem.numjumps, mem.traveldist )
   if mem.timelimit < playerbest then
      if not vntk.yesno( _("Too slow"), fmt.f(
            _("This shipment must arrive within {time_limit}, but it will take at least {time} for your ship to reach {pnt}, missing the deadline. Accept the mission anyway?"),
	    {time_limit=(mem.timelimit - time.get()), time=(playerbest - time.get()), pnt=mem.destplanet} ) ) then
         return
      end
   end
   if player.pilot():cargoFree() < mem.amount then
      vntk.msg( _("No room in ship"), fmt.f(
         _("You don't have enough cargo space to accept this mission. It requires {tonnes_free} of free space ({tonnes_short} more than you have)."),
         { tonnes_free = fmt.tonnes(mem.amount), tonnes_short = fmt.tonnes( mem.amount - player.pilot():cargoFree() ) } ) )
      return
   end

   misn.accept()

   mem.carg_id = misn.cargoAdd( mem.cargo, mem.amount )
   vntk.msg( _("Mission Accepted"), fmt.f(
      _("{tonnes} of {cargo} are loaded onto your ship."),
      {tonnes=fmt.tonnes(mem.amount), cargo=_(mem.cargo)} ) )
   hook.land( "land" ) -- only hook after accepting
   hook.date(time.new(0, 0, 100), "tick") -- 100STU per tick
   tick() -- set OSD
end

-- Land hook
function land()
   if spob.cur() == mem.destplanet then
         vntk.msg( _("Successful Delivery"), fmt.f(
            _("The containers of {cargo} are unloaded at the docks."), {cargo=_(mem.cargo)} ) )
      player.pay(mem.reward)
      local n = var.peek("ps_misn") or 0
      var.push("ps_misn", n+1)

      -- increase faction
      faction.modPlayerSingle(mem.reward_faction, rnd.rnd(2, 4))
      misn.finish(true)
   end
end

-- Date hook
function tick()
   if mem.timelimit >= time.get() then
      -- Case still in time
      misn.osdCreate(fmt.f(_("Smuggling {cargo}"), {cargo=_(mem.cargo)}), {
         fmt.f(_("Fly to {pnt} in the {sys} system before {time_limit}\n({time} remaining)"),
               {pnt=mem.destplanet, sys=mem.destsys, time_limit=mem.timelimit, time=(mem.timelimit - time.get())})
      })
   elseif mem.timelimit <= time.get() then
      -- Case missed deadline
      lmisn.fail(_("You have failed to deliver the goods on time!"))
   end
end
