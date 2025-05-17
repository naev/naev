local vn = require "vn"
local fmt = require "format"
local tut = require "common.tutorial"

local lib = {}

local REGROW_TIME = time.new( 0, 1200, 0 )
local HARVEST_AMOUNT = 2
local CROP = commodity.get("Astral Nectar")
lib.SPOBS = {
   spob.get("Unicorn II"),
   spob.get("Chloe I"),
   spob.get("Nougat II"),
   spob.get("Quai IIb"),
}
local VARNAMES = {
   "astral_orchids_unicorn_ii",
   "astral_orchids_chloe_i",
   "astral_orchids_nougat_ii",
   "astral_orchids_quai_iib",
}

-- Assume it's an event
-- luacheck: globals evt

function lib.setup( params )
   params = params or {}

   local ao = {}
   ao.regrow_time = params.regrow_time or REGROW_TIME
   ao.harvest_amount = params.harvest_amount or HARVEST_AMOUNT

   mem._astral_orchids = ao

   create = lib.create -- luacheck: globals create
end

function lib.create()
   -- Get the ID of the spob
   local id = 0
   for k,v in ipairs(lib.SPOBS) do
      if v==spob.cur() then
         id = k
         break
      end
   end
   if spob==0 then
      return error(fmt.f("Astral Orchid event occurring on unknown spob '{spb}'!",
         {spb=spob.cur()}))
   end
   local varname = VARNAMES[id]

   local ao = mem._astral_orchids
   local lastpicked = var.peek( varname )
   local canpick = (not lastpicked) or (lastpicked >= time.get() + ao.regrow_time)
   local didpoi = var.peek( "poi_orchids" ) > 0
   local orchids_found = 0
   for k,v in ipairs(player.evtDoneList) do
      if v.tags.astral_orchids then
         orchids_found = orchids_found + 1
      end
   end

   vn.clear()
   vn.scene()

   if not canpick then
      -- Sorry player
      vn.transition()
      vn.na(_([[You land and go check on the Astral Orchids you found last times. It seems like not enough time has passed for the nectar to regrow.]]))
      vn.done()
      vn.run()
      return evt.finish()
   elseif lastpicked~=nil then
      -- Skip intro stuff
      vn.transition()
      vn.na(_([[You land and go check on the Astral Orchids you found last time. It seems like they are ready to harvest again.]]))
      vn.menu{
         {_([[Harvest the Orchids.]]), "02_harvest"},
         {_([[Let them be.]]), "02_leave"},
      }
   end

   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   vn.na(fmt.f(_([[Your ship lands, and you run a preliminary scan, when {ainame} pops up.]]),
      {ainame=tut.ainame()}))

   if id==1 then -- Unicorn II
      sai(_([["I have detected some organic material in a nearby cave which may be of interest. Or not. I lack the statistical priors to estimate your response to such data."]]))

      vn.menu{
         {_([[Explore the caves.]]), "01_explore"},
         {_([[Maybe next time.]]), "01_maybenext"},
      }

      vn.label("01_explore")
      vn.na(fmt.f(_([[You don your atmospheric suit and take a weapon with you as you leave your ship, just in case. You head towards the coordinates provided by {ainame}.]]),
            {ainame=tut.ainame()}))
      vn.na(_([[You find the entrance to a cave nearby, and start exploring the subterranean world. Eventually, you find an expansive cavern, and your sensors pick up nearby organic matter. Looking carefully, you find some small plants that seem to be flowering, what are tho odds?]]))
   elseif id==2 then -- Chloe I
      sai(_([["I have detected an unusual structure nearby that may be of interest."]]))
      vn.menu{
         {_([[Explore the structure.]]), "01_explore"},
         {_([[Maybe next time.]]), "01_maybenext"},
      }

      vn.label("01_explore")
      vn.na(fmt.f(_([[You put on an atmosphere suit, pick up a weapon, and head out towards the coordinates provided by {ainame}.]]),
            {ainame=tut.ainame()}))
      vn.na(_([[You walk around, pushing your way through the vegetation, and eventually your scanner picks up some large metal object. You have to dig through the vegetation and eventually find some sort of ancient rusted hatch.]]))
      vn.menu{
         {_([[Open the hatch.]]), "02_open"},
         {_([[Leave it be.]]), "02_leave"},
      }

      vn.label("02_leave")
      vn.na(_([[You leave the hatch and return to your ship. Maybe some things are left unknown.]]))
      vn.done( tut.shipai.transition )

      vn.label("02_open")
      vn.na(_([[It takes quite a bit of effort, but you are eventually able to pry the hatch open. It seems to lead down into a man-made cave.]]))
      vn.na(_([[You make your way into it, happy to be unobstructed by denser vegetation, and find yourself surrounded by small flowering plants.]]))
   elseif id==3 then -- Nougat II
      sai(_([["There seems to be an anomaly in the planetary surface nearby. You might wish to investigate."]]))
      vn.menu{
         {_([[Explore the surface anomaly.]]), "01_explore"},
         {_([[Maybe next time.]]), "01_maybenext"},
      }

      vn.label("01_explore")
      vn.na(fmt.f(_([[You stretch a bit, don your gear, and get ready to head out into the eternal snowstorm that is Nougat II towards the coordinates provided by {ainame}.]]),
            {ainame=tut.ainame()}))
      vn.na(_([[You trudge along and eventually find a mount of ice and snow that seems to be mainly hollow. You set your weapon to cook and melt a passage that seems to lead to an interior cavern.]]))
      vn.na(_([[As you make your way, you notice that the temperature goes up nearly a hundred degrees to something somewhat acceptable. You also notice that there seems to be many small flowering plants nearby.]]))
   elseif id==4 then -- Quai IIb
      sai(_([["While I admire your faith in this spaceship to land on such a toxic environment, I would like to point out that there seems to be something strange picked up on the sensor nearby. You may be interested in further enjoying the toxic sludge with a stroll to check it out."]]))
      vn.menu{
         {_([[Explore the strange area and enjoy a stroll in the toxic sludge.]]), "01_explore"},
         {_([[Maybe next time.]]), "01_maybenext"},
      }

      vn.label("01_explore")
      vn.na(fmt.f(_([[You don your extra-protective atmospheric suit, hope that it survives the local environment, and head off to the coordinates provided by {ainame}.]]),
            {ainame=tut.ainame()}))
      vn.na(fmt.f(_([[Eventually you find what seems to be a cave blocked by a large boulder, potentially a few hundred kilograms. Making full use of the low gravity on {spb}, you roll it a bit out of the way and enter the cave, which seems to have been shielded from the toxic sludge.]]),
         {spb=spob.cur()}))
      vn.na(_([[You explore around and find that it seems to be covered in lots of cute small flowering plants.]]))
   else
      return error(fmt.f("No vn text for Astral Orchids event on spob '{spb}'!", {spb=spob.cur()}))
   end
   vn.jump("01_cont")

   vn.label("01_maybenext")
   vn.done()

   vn.label("01_cont")
   local cropname = "#b".._("Astral Orchids").."#0"
   local cropmsg = fmt.f(_([["These seem to be {crop}, a rare species that requires specific conditions to thrive, and do rely on chemosynthesis for survival. They are quite rare and highly sought after by gourmets for their nectar."]]),
         {crop=cropname})
   if didpoi then
      if orchids_found > 0 then
         local left = #lib.SPOBS-(orchids_found+1)
         if left <= 0 then
            sai(fmt.f(_([["It looks like you found another cluster of {crop}. This should be the last of the locations that were planted according to the derelict we found."]]),
               {crop=cropname}))
         else
            local spobs_left = {}
            for k,v in ipairs( lib.SPOBS ) do
               if v~=spob.cur() and (not var.peek( VARNAMES[k] )) then
                  table.insert( spobs_left, v )
               end
            end
            sai(fmt.f(_([["It looks like you found another cluster of {crop}. Including this, you have found {num} planted areas. There should be {left} locations left, in particular, {left}."]]),
               {crop=cropname, num=orchids_found+1, left="#b"..fmt.list(spobs_left).."#0"}))
         end
      else
         sai(cropmsg)
         sai(fmt.f(_([["However, the plant was deemed to be extinct. This may be related to the derelict we found that mentioned planting Astral Orchids at {locations} systems."]]),
            {locations=fmt.list(lib.SPOBS)}))
      end
   else
      if orchids_found > 0 then
         sai(fmt.f(_([["It seems like you found yet another cluster of {crop}, making it a total of {num}. This seems statistically unlikely for a plant deemed extinct. Maybe the different locations are related with a similar underlying cause? I shall revise my databases for additional information."]]),
            {crop=cropname, num=orchids_found+1}))
      else
         sai(cropmsg)
         sai(_([["However, the plant was deemed to be extinct. It is incredible that you were able to find wild ones growing in such a remote location."]]))
      end
   end
   vn.na(_([[What do you want to do?]]))
   vn.menu{
      {_([[Harvest the Orchids.]]), "02_harvest"},
      {_([[Let them be.]]), "02_leave"},
   }

   vn.label("02_leave")
   vn.na(_([[You let the Astral Orchids be, and make your way back to your ship. Some things are best left alone.]]))
   vn.done( tut.shipai.transition )

   vn.label("02_harvest")
   local harvested = 0
   vn.func( function ()
      if player.fleetCargoFree() <= 0 then
         return vn.jump("nofit")
      end
      harvested = player.fleetCargoAdd( CROP, ao.harvest_amount )
      var.push( varname, time.get() )
      time.inc( time.new( 0, 1, 0 ) ) -- Small time delay, TODO make visible to the player?
   end )
   vn.na( function ()
      return fmt.f(_([[You carefully collect {amount} of {crop} and bring it back to your ship. It seems like it'll likely take a while to grow back.]]),
         {amount=fmt.tonnes(harvested), crop=CROP})
   end )
   vn.done( tut.shipai.transition )

   vn.label("nofit")
   vn.na(_([[You realize you don't have the cargo space to fit what you could collect. It looks like you'll have to come back later.]]))
   if lastpicked==nil then
      vn.done()
   else
      vn.done( tut.shipai.transition )
   end

   vn.run()

   evt.finish( harvested > 0 )
end

return lib
