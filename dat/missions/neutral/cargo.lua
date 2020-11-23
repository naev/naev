--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Cargo">
  <avail>
   <priority>6</priority>
   <chance>960</chance>
   <location>Computer</location>
   <faction>Dvaered</faction>
   <faction>Empire</faction>
   <faction>Frontier</faction>
   <faction>Goddard</faction>
   <faction>Independent</faction>
   <faction>Proteron</faction>
   <faction>Sirius</faction>
   <faction>Soromid</faction>
   <faction>Thurion</faction>
   <faction>Traders Guild</faction>
   <faction>Za'lek</faction>
  </avail>
  <notes>
   <tier>1</tier>
  </notes>
 </mission>
 --]]
--[[
   -- These are regular cargo delivery missions. Pay is low, but so is difficulty.
   -- Most of these missions require BULK ships. Not for small ships!
--]]

require "cargo_common.lua"
require "numstring.lua"


misn_desc = {}
-- Note: indexed from 0 to match mission tiers.
misn_desc[0] = _("Small shipment to %s in the %s system.")
misn_desc[1] = _("Medium shipment to %s in the %s system.")
misn_desc[2] = _("Sizable cargo delivery to %s in the %s system.")
misn_desc[3] = _("Large cargo delivery to %s in the %s system.")
misn_desc[4] = _("Bulk freight delivery to %s in the %s system.")

misn_details = _([[
Cargo: %s (%s)
Jumps: %d
Travel distance: %d
%s]])

piracyrisk = {}
piracyrisk[1] = _("Piracy Risk: None")
piracyrisk[2] = _("Piracy Risk: Low")
piracyrisk[3] = _("Piracy Risk: Medium")
piracyrisk[4] = _("Piracy Risk: High")
--=Landing=--

cargo_land_title = _("Delivery success!")

cargo_land = {}
cargo_land[1] = _("The containers of %s are carried out of your ship by a sullen group of workers. The job takes inordinately long to complete, and the leader pays you without speaking a word.")
cargo_land[2] = _("The containers of %s are rushed out of your vessel by a team shortly after you land. Before you can even collect your thoughts, one of them presses a credit chip in your hand and departs.")
cargo_land[3] = _("The containers of %s are unloaded by an exhausted-looking bunch of dockworkers. Still, they make fairly good time, delivering your pay upon completion of the job.")
cargo_land[4] = _("The containers of %s are unloaded by a team of robotic drones supervised by a human overseer, who hands you your pay when they finish.")

osd_title = _("Cargo mission")
osd_msg = _("Fly to %s in the %s system")

-- Create the mission
function create()
   -- Note: this mission does not make any system claims. 
   
   -- Calculate the route, distance, jumps, risk of piracy, and cargo to take
   destplanet, destsys, numjumps, traveldist, cargo, avgrisk, tier = cargo_calculateRoute()
   if destplanet == nil then
      misn.finish(false)
   end
   
   if avgrisk == 0 then
      piracyrisk = piracyrisk[1]
      riskreward = 0
   elseif avgrisk <= 25 then
      piracyrisk = piracyrisk[2]
      riskreward = 10
   elseif avgrisk > 25 and avgrisk <= 100 then
      piracyrisk = piracyrisk[3]
      riskreward = 25
   else
      piracyrisk = piracyrisk[4]
      riskreward = 50
   end

   -- Choose amount of cargo and mission reward. This depends on the mission tier.
   -- Reward depends on type of cargo hauled. Hauling expensive commodities gives a better deal.
   -- Note: Pay is independent from amount by design! Not all deals are equally attractive!
   finished_mod = 2.0 -- Modifier that should tend towards 1.0 as Naev is finished as a game
   amount = rnd.rnd(5 + 25 * tier, 20 + 60 * tier)
   jumpreward = commodity.price(cargo)
   distreward = math.log(100*commodity.price(cargo))/100
   reward = 1.5^tier * (avgrisk*riskreward + numjumps * jumpreward + traveldist * distreward) * finished_mod * (1. + 0.05*rnd.twosigma())

   misn.setTitle( _("Shipment to %s in %s (%s)"):format(
         _(destplanet:name()), _(destsys:name()), tonnestring(amount) ) )
   misn.markerAdd(destsys, "computer")
   misn.setDesc(
      misn_desc[tier]:format( _(destplanet:name()), _(destsys:name()) ) .. "\n\n"
      .. misn_details:format(
         _(cargo), tonnestring(amount), numjumps, traveldist, piracyrisk ) )
   misn.setReward( creditstring(reward) )

end

-- Mission is accepted
function accept()
   if player.pilot():cargoFree() < amount then
      tk.msg( _("No room in ship"), string.format(
         _("You don't have enough cargo space to accept this mission. It requires %s of free space (%s more than you have)."),
         tonnestring(amount),
         tonnestring( amount - player.pilot():cargoFree() ) ) )
      misn.finish()
   end
   misn.accept()
   misn.cargoAdd(_(cargo), amount) -- TODO: change to jettisonable cargo once custom commodities are in. For piracy purposes.
   misn.osdCreate(osd_title, {osd_msg:format(_(destplanet:name()), _(destsys:name()))})
   hook.land("land")
end

-- Land hook
function land()
   if planet.cur() == destplanet then
      -- Semi-random message.
      tk.msg( cargo_land_title, cargo_land[rnd.rnd(1, #cargo_land)]:format(_(cargo)) )
      player.pay(reward)
      misn.finish(true)
   end
end

function abort ()
   misn.finish(false)
end

