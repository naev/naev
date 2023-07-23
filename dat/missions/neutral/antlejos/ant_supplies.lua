--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Supplies for Antlejos">
 <priority>4</priority>
 <chance>350</chance>
 <location>Computer</location>
 <spob>Antlejos V</spob>
 <done>Terraforming Antlejos 5</done>
 <notes>
  <campaign>Terraforming Antlejos</campaign>
 </notes>
</mission>
--]]
--[[
   Campaign to Terraform Antlejos V

   Bring in more volunteers and materials to Verner.
   Have encounters with the PUAAA
--]]
local vn = require "vn"
local vntk = require "vntk"
local fmt = require "format"
local ant = require "common.antlejos"
local fleet = require "fleet"
local car = require "common.cargo"

local cargo_name = _("Volunteers and Equipment")

local returnpnt, returnsys = spob.getS("Antlejos V")

local levelup = {
   0,
   0,
   0,
   0,
   0, -- bar, missions
   -- Up to 5 already done
   1000, -- +commodity
   1500, -- +bad outfits
   2000, -- +better outfits
   2500, -- +bad spaceships
}


function create ()
   mem.tier = rnd.rnd(1,3)

   -- Inclusive claiming, multiple missions can be done at the same time!
   if not misn.claim( returnsys, true ) then misn.finish( false ) end

   mem.destpnt, mem.destsys, mem.numjumps, mem.traveldist = car.calculateRoute( rnd.rnd(3,5)+mem.tier, true )
   if not mem.destpnt then
      misn.finish(false)
      return
   end
   mem.amount = 50 * math.pow(mem.tier,2)
   mem.reward = (50e3 + mem.tier * 25e3 + mem.numjumps * 10e3 + mem.traveldist * 0.2) * ((mem.tier-1)/5 + 1 + 0.05*rnd.twosigma())

   local adj = p_("adj delivery", "Small")
   if mem.tier==2 then
      adj = p_("adj delivery", "Medium")
   elseif mem.tier==3 then
      adj = p_("adj delivery", "Large")
   end
   misn.setTitle( fmt.f(_("#oANTLEJOS:#0 {adj} delivery of supplies from {pnt} ({sys} system)"),
         {adj=adj, pnt=mem.destpnt, sys=mem.destsys}) )
   misn.setReward(mem.reward)

   local desc = fmt.f(_("Pick up {cargo} at {pnt} in the {sys} system and bring them to {retpnt} to further accelerate the terraforming. Note that protestors from the PUAAA are expected to show up.\n"),
      {cargo=cargo_name, pnt=mem.destpnt, sys=mem.destsys, retpnt=returnpnt})
   desc = desc .. "\n" .. fmt.f( _("#nCargo:#0 {cargo} ({mass})"), {cargo=cargo_name, mass=fmt.tonnes(mem.amount)} )
   desc = desc .. "\n" .. fmt.f( n_("#nJumps:#0 {jumps}", "#nJumps:#0 {jumps}", mem.numjumps ), {jumps=mem.numjumps} )
   desc = desc .. "\n" .. fmt.f( n_("#nTravel distance:#0 {dist}", "#nTravel distance:#0 {dist}", mem.traveldist), {dist=fmt.number(mem.traveldist)} )
   local total = ant.supplied_total()
   desc = desc .. "\n" .. fmt.f( _("#nSupplies at Antlejos V:#0 {total}"), {total=fmt.tonnes(total)} )
   local level = ant.unidiffLevel()
   if level < #levelup then
      local needed = levelup[ level+1 ] - total
      if needed > 0 then
         desc = desc .. "\n" .. fmt.f( _("#nSupplies until next upgrade:#0 {needed}"), {needed=fmt.tonnes(needed)} )
      end
   end
   misn.setDesc( desc )
   mem.mrk = misn.markerAdd( mem.destpnt )
end

function accept()
   misn.accept()
   misn.osdCreate(_("Supplies for Antlejos"), {
      fmt.f(_("Pick up the {cargo} at {pnt} ({sys} system)"), {cargo=cargo_name, pnt=mem.destpnt, sys=mem.destsys}),
      fmt.f(_("Deliver the cargo to {pnt} ({sys} system)"), {pnt=returnpnt, sys=returnsys})
   })
   mem.state = 1

   hook.land( "land" )
   hook.enter( "enter" )
end

-- Land hook.
function land ()
   if mem.state==1 and  spob.cur() == mem.destpnt then

      local fs = player.pilot():cargoFree()
      if fs < mem.amount then
         vntk.msg(_("Insufficient Space"), fmt.f(_("You have insufficient free cargo space for the {cargo}. You only have {freespace} of free space, but you need at least {neededspace}."),
            {cargo=cargo_name, freespace=fmt.tonnes(fs), neededspace=fmt.tonnes(mem.amount)}))
         return
      end
      vntk.msg(_("Cargo Loaded"), _("The volunteers load the equipment onto your ship and get onboard.") )

      local c = commodity.new( N_("Volunteers and Equipment"), N_("Volunteers and equipment meant for terraforming Antlejos V.") )
      misn.cargoAdd( c, mem.amount )
      misn.osdActive(2)
      mem.state = 2

      misn.markerMove( mem.mrk, returnpnt )

   elseif mem.state==2 and spob.cur() == returnpnt then
      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(_([[The volunteers unload the equipment and get off your ship. It looks like they are eager to get started.]]))
      vn.sfxVictory()
      vn.na( fmt.reward(mem.reward) )
      vn.run()

      ant.supplied( mem.amount )

      player.pay( mem.reward )
      ant.log(fmt.f(_("You delivered {amount} of {cargo} to {pnt} for terraforming."),{amount=mem.amount, cargo=cargo_name, pnt=returnpnt}))

      -- Level up antlejos as necessary
      local level = ant.unidiffLevel()
      local total = ant.supplied_total()
      for l = 1,#levelup do
         if level < l and total >= levelup[l] then
            -- TODO better messages
            vntk.msg(_("Terraforming Progress"),_("Through the new supplies and volunteers, Antlejos V has been able to expand its facilities."))
            ant.unidiff( ant.unidiff_list[level+1] )
            break
         end
      end

      misn.finish(true)
   end
end

local plts
function enter ()
   if mem.state~=2 or system.cur() ~= returnsys then
      return
   end

   pilot.clear()
   pilot.toggleSpawn(false)

   local puaaa = ant.puaaa()

   local ships = "Hyena"
   local shipnum = 2
   if mem.tier==2 then
      ships = { "Lancelot", "Vendetta", "Shark" }
      shipnum = 1
   elseif mem.tier==3 then
      ships = { "Admonisher", "Lancelot", "Lancelot" }
      shipnum = 1
   end

   plts = fleet.add( shipnum, ships, puaaa, returnpnt:pos(), _("Protestor"), {ai="guard"} )
   for _k,p in ipairs(plts) do
      p:setVisplayer()
   end

   hook.timer( 10, "protest" )
end

local protest_id, attacked
function protest ()
   if protest_id == nil then
      protest_id = rnd.rnd(1,#ant.protest_lines)
   end

   -- See surviving pilots
   local nplts = {}
   for _k,p in ipairs(plts) do
      if p:exists() then
         table.insert( nplts, p )
      end
   end
   plts = nplts
   if #plts <= 0 then
      return
   end

   local p = plts[ rnd.rnd(1,#plts) ]
   if not attacked and p:inrange( player.pilot() ) then
      attacked = true
      for _k, pk in ipairs(plts) do
         pk:setHostile()
      end
      p:broadcast( fmt.f(_("Hey! That ship is helping to terraform {pnt}! Get them!"),{pnt=returnpnt}) )
      player.autonavReset(5)
   else
      -- Say some protest slogan
      p:broadcast( ant.protest_lines[ protest_id ] )
      protest_id = math.fmod(protest_id, #ant.protest_lines)+1
   end

   -- Protest again
   hook.timer( 15, "protest" )
end
