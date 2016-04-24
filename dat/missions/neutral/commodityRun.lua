--[[
-- Commodity delivery missions. Player receives a bonus for bringing more of a commodity back.
--]]

include "dat/scripts/numstring.lua"

lang = naev.lang()
if lang == "es" then
else -- default english

    --Mission Details
    misn_title = "%s Delivery"
    misn_desc = "%s in the %s system needs a delivery of %s."
    misn_reward = "%s credits/ton plus a bonus."
      
    --accept_title = "Mission Accepted"
    --accept_text = "We're in short supply of %s. Find a station that sells %s and bring back as much as your ship can hold. We give bigger bonuses for bigger loads. Don't come back without it!"
    
    osd_title = "Commodity Delivery"
    osd_msg = "Delivery of %s to %s in the %s system."
       --=Landing=--change this

   cargo_land_title = "Delivery success!"

   cargo_land_p1 = {}
   cargo_land_p1[1] = "The crates of "
   cargo_land_p1[2] = "The drums of "
   cargo_land_p1[3] = "The containers of "

   cargo_land_p2 = {}
   cargo_land_p2[1] = " are carried out of your ship and tallied. After several different men double-check the register to confirm the amount, you paid %s credits and summarily dismissed."
   cargo_land_p2[2] = " are quickly and efficiently unloaded, labeled, and readied for distribution. The delivery manager thanks you with a credit chip worth %s credits."
   cargo_land_p2[3] = " are unloaded from your vessel by a team of dockworkers who are in no rush to finish, eventually delivering %s credits after the number of tons is determined."
   cargo_land_p2[4] = " are unloaded by robotic drones that scan and tally the contents. The human overseerer hands you %s credits when they finish."

    cargo_land_title_fail = "Mission Failed!"
    
    cargo_land_fail = "The delivery manager is visibly upset with you. 'What is this?! You're missing the %s per the contract! This is unacceptable. We will find a more competant captain to complete this mission.' He shakes his head and walks off."


end

--Create the mission
function create()
    -- Note: this mission does not make any system claims.
  
    ReturnPlanet = planet.cur()
    ReturnSystem = system.cur()
	
	PossibleCommodities = planet.commoditiesSold("Darkshed") --TODO: find a better way to index all available commodities
	Commodity = PossibleCommodities[rnd.rnd(1,#PossibleCommodities)]
	for k,v in pairs(planet.cur():commoditiesSold()) do
		if v == Commodity then
  			misn.finish(false)
  		end
	end

    --Set Mission Details
    misn.setTitle(misn_title:format(Commodity:name()))
    misn.markerAdd(system.cur(), "computer")
    misn.setDesc(misn_desc:format(planet.cur():name(),system.cur():name(),Commodity:name()))
    misn.setReward(misn_reward:format(commodity.price(Commodity:name())))
    
end

-- Mission is accepted
function accept()

    misn.accept() -- I think this is where the tk.msg goes?
    misn.osdCreate(osd_title, {osd_msg:format(Commodity:name(),ReturnPlanet:name(), ReturnSystem:name())})
    hook.land("land")
end

-- Land hook
function land()
    --Does the player have the right cargo? If so, how much?
    amount = pilot.cargoHas(player.pilot(),Commodity:name())  
    --Determine bonus for amount delivered. 
    if amount <= 20 then
        Bonus = 1.05 -- 5% bonus for <20 tons
    elseif amount > 20 and amount <= 40 then
        Bonus = 1.10 -- 10% bonus for 20 - 40 tons
    elseif amount > 40 and amount <= 60 then
        Bonus = 1.15 -- 15% bonus for 40 - 60 tons
    elseif amount > 60 and amount <= 80 then
        Bonus = 1.20 -- 20% bonus for 60 - 80 tons
    elseif amount > 80 and amount <= 100 then
        Bonus = 1.25 -- 25% bonus for 80 - 100 tons
    elseif amount >100 and amount <= 150 then
        Bonus = 1.30 -- 30% bonus for 100 - 150 tons
    elseif amount > 150 and amount <= 200 then
        Bonus = 1.35 -- 35% bonus for 150 - 200 tons
    elseif amount > 200 and amount <= 250 then
        Bonus = 1.40 -- 40% bonus for 150 - 200 tons
    elseif amount > 250 and amount <= 300 then
    	Bonus = 1.45 -- 45% bonus for 250 - 300 tons
    else
    	Bonus = 1.5 -- 50% bonus for 300+ tons
    end
    
    --Reward
    finished_mod = 2.0 -- Modifier that should tend towards 1.0 as naev is finished as a game
    reward =  finished_mod * (commodity.price(Commodity:name()) * amount * Bonus)
    
    if planet.cur() == ReturnPlanet and amount > 0 then      
        tk.msg(cargo_land_title, cargo_land_p1[rnd.rnd(1, #cargo_land_p1)] .. Commodity:name() .. cargo_land_p2[rnd.rnd(1, #cargo_land_p2)]:format(numstring(reward)))
        pilot.cargoRm(player.pilot(),Commodity:name(),amount)
        player.pay(reward)
        misn.finish(true)
    else
    	tk.msg(cargo_land_title_fail, cargo_land_fail:format(Commodity:name()))
    	misn.finish(true)
    end
end

function abort ()
    misn.finish(false)
end

