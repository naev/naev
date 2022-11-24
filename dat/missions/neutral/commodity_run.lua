--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Commodity Run">
 <priority>5</priority>
 <chance>90</chance>
 <location>Computer</location>
 <cond>
   local cra = var.peek("commodity_runs_active") or 0
   if cra &gt;= 3 then
      return false
   end
   local f = spob.cur():faction()
   if f then
      local ft = f:tags()
      if ft.generic or ft.misn_cargo then
         return true
      end
   end
   return false
 </cond>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[
   Commodity delivery missions.
--]]
local pir = require "common.pirate"
local fmt = require "format"
local vntk = require "vntk"

-- luacheck: globals cargo_land commchoices (shared with derived missions flf.commodity_run, pirate.pirate_commodity_run)
-- luacheck: globals enter land (Hook functions passed by name)

--Mission Details
mem.misn_title = _("{cargo} Delivery")
mem.misn_desc = _("{pnt} has an insufficient supply of {cargo} to satisfy the current demand. Go to any planet which sells this commodity and bring as much of it back as possible.")

cargo_land = {
   _("The containers of {cargo} are carried out of your ship and tallied. After several different workers double-check the register to confirm the amount, you are paid {credits} and summarily dismissed."),
   _("The containers of {cargo} are quickly and efficiently unloaded, labeled, and readied for distribution. The delivery manager thanks you with a credit chip worth {credits}."),
   _("The containers of {cargo} are unloaded from your vessel by a team of dockworkers who are in no rush to finish, eventually delivering {credits} after the number of tonnes is determined."),
   _("The containers of {cargo} are unloaded by robotic drones that scan and tally the contents. The human overseer hands you {credits} when they finish."),
}

mem.osd_title = _("Commodity Delivery")
mem.paying_faction = faction.get("Independent")


-- A script may require "missions.neutral.commodity_run" and override this
-- with a table of (raw) commodity names to choose from.
commchoices = nil


local function update_active_runs( change )
   local current_runs = var.peek( "commodity_runs_active" )
   if current_runs == nil then current_runs = 0 end
   var.push( "commodity_runs_active", math.max( 0, current_runs + change ) )

   -- Note: This causes a delay (defined in create()) after accepting,
   -- completing, or aborting a commodity run mission. This is
   -- intentional.
   var.push( "last_commodity_run", time.tonumber( time.get() ) )
end


function create ()
   -- Note: this mission does not make any system claims.
   mem.misplanet, mem.missys = spob.cur()

   if commchoices == nil then
      local std = commodity.getStandard();
      mem.chosen_comm = std[rnd.rnd(1, #std)]:nameRaw()
   else
      mem.chosen_comm = commchoices[rnd.rnd(1, #commchoices)]
   end
   local comm = commodity.get(mem.chosen_comm)
   local mult = 1 + math.abs(rnd.twosigma() * 2)
   mem.price = comm:price() * mult

   local last_run = var.peek( "last_commodity_run" )
   if last_run ~= nil then
      local delay = time.new(0, 7, 0)
      if time.get() < time.fromnumber(last_run) + delay then
         misn.finish(false)
      end
   end

   for _i, j in ipairs( mem.missys:spobs() ) do
      for _k, v in pairs( j:commoditiesSold() ) do
         if v == comm then
            misn.finish(false)
         end
      end
   end

   -- Set Mission Details
   misn.setTitle( fmt.f( mem.misn_title, {cargo=comm} ) )
   misn.markerAdd( mem.misplanet, "computer" )
   misn.setDesc( fmt.f( mem.misn_desc, {pnt=mem.misplanet, cargo=comm} ) )
   misn.setReward( fmt.f(_("{credits} per tonne"), {credits=fmt.credits(mem.price)} ) )
end


function accept ()
   local comm = commodity.get(mem.chosen_comm)

   misn.accept()
   update_active_runs( 1 )

   misn.osdCreate(mem.osd_title, {
      fmt.f(_("Buy as much {cargo} as possible"), {cargo=comm} ),
      fmt.f(_("Take the {cargo} to {pnt} in the {sys} system"), {cargo=comm, pnt=mem.misplanet, sys=mem.missys} ),
   })

   hook.enter("enter")
   hook.land("land")
end


function enter ()
   if player.fleetCargoOwned( mem.chosen_comm ) > 0 then
      misn.osdActive(2)
   else
      misn.osdActive(1)
   end
end


function land ()
   local amount = player.fleetCargoOwned( mem.chosen_comm )

   if spob.cur() == mem.misplanet and amount > 0 then
      amount = player.fleetCargoRm( mem.chosen_comm, amount )
      local reward = amount * mem.price
      local txt = fmt.f(cargo_land[rnd.rnd(1, #cargo_land)],
            {cargo=_(mem.chosen_comm), credits=fmt.credits(reward)} )
      vntk.msg(_("Delivery success!"), txt)
      player.pay(reward)
      if not pir.factionIsPirate( mem.paying_faction ) then
         pir.reputationNormalMission(rnd.rnd(2,3))
      end
      update_active_runs(-1)
      misn.finish(true)
   end
end


function abort ()
   update_active_runs(-1)
end
