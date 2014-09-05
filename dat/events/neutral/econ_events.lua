--[[

   Economy Events

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]

include "jumpdist.lua"


events = {}

events[1] = {}
events[1]["time"] = 150
events[1][1] = { "Food", 3.0 }

events[2] = {}
events[2]["time"] = 100
events[2][1] = { "Food", 0.5 }

events[3] = {}
events[3]["time"] = 50
events[3][1] = { "Industrial Goods", 1.5 }
events[3][2] = { "Ore", 1.5 }

events[4] = {}
events[4]["time"] = 25
events[4][1] = { "Luxury Goods", 1.5 }

events[5] = {}
events[5]["time"] = 100
events[5][1] = { "Medicine", 2 }


lang = naev.lang()
if lang == 'es' then --not translated atm
else --default english
   events[1]["title"] = "Famine on %s"
   events[1]["text"] = "The planet %s is experiencing a famine due to an unexpected food shortage. Food prices have skyrocketed as a result."

   events[2]["title"] = "Bumper Crop on %s"
   events[2]["text"] = "More crops than usual are being harvested on the planet %s, resulting in lower food prices."

   events[3]["title"] = "Worker Strike on %s"
   events[3]["text"] = "Prices of industrial goods and ore have risen on the planet %s due to a worker's strike."

   events[4]["title"] = "Cat Convention on %s"
   events[4]["text"] = "Many wealthy people are visiting the planet %s for a cat convention, resulting in a high demand for luxury goods."

   events[5]["title"] = "Disease Outbreak on %s"
   events[5]["text"] = "The demand for medicine on the planet %s has spiked due to an outbreak of an unpleasent disease."
end


function create ()
   --get the event
   local event_num = rnd.rnd( 1, #events )
   local event = events[event_num]

   --get a planet with no current events
   local economic_articles = news.get( "economic events" )
   local systems = getsysatdistance( system.cur(), 0, 5 )
   while event_planet == nil and #systems > 0 do
      i = rnd.rnd( 1, #systems )
      local sys = systems[i]
      for j = i, #systems - 1 do
         systems[j] = systems[j + 1]
      end
      systems[#systems] = nil

      local planets = sys:planets()
      while event_planet == nil and #planets > 0 do
         local i = rnd.rnd( 1, #planets )
         local p = planets[i]
         for j = i, #planets - 1 do
            planets[j] = planets[j + 1]
         end
         planets[#planets] = nil

         local planet_works = false
         if p:services()["commodity"] and
               not planet.cur():faction():areEnemies( p:faction() ) then
            planet_works = true

            for i = 1, #economic_articles do
               if string.match( economic_articles[i]:title(), p:name() ) then
                  planet_works = false
                  break
               end
            end

            local commodities_sold = p:commoditiesSold()
            for i = 1, #event do
               local has_commodity = false
               for j = 1, #commodities_sold do
                  if commodities_sold[j]:name() == event[i][1] then
                     has_commodity = true
                     break
                  end
               end
               if not has_commodity then
                  planet_works = false
                  break
               end
            end
         end

         if planet_works then
            event_planet = p
            break
         end
      end
   end

   if event_planet ~= nil then
      original_prices = {}
      for i = 1, #event do
         local good = commodity.get( event[i][1] )
         if econ.isPlanetPriceSet( good, event_planet ) then
            original_prices[good:name()] = econ.getPrice( good, event_planet )
         end

         local price = econ.getPrice( good, event_planet ) * event[i][2]
         econ.setPlanetPrice( good, event_planet, price )
      end

      --update the prices, and make the article
      econ.updatePrices()
      make_article( event )

      --set up the event ending
      hook.date( time.create(0, event["time"], 0), "end_event" )
      evt.save(true)
   end
end


   --make the news event for the selected event and system
function make_article( event )
   local title = event["title"]:format( event_planet:name() )
   local body = event["text"]:format( event_planet:name() )
   local article = news.add( "Generic", title, body,
      time.get() + time.create( 0, event["time"], 0 ) )
   article:bind("economic event")
end


--end the event, and return the values to their original values
function end_event ()
   local commodities = event_planet:commoditiesSold()

   --reset prices to their original states
   for i = 1, #commodities do
      local c = commodities[i]
      local price = original_prices[c:name()]
      if price ~= nil then
         econ.setPlanetPrice( c, event_planet, price )
      else
         econ.unsetPlanetPrice( c, event_planet )
      end
   end

   evt.finish()
end

