local function test_hypergates_construction ()
   return player.chapter()=="0"
end
local function test_hypergates_done ()
   return player.chapter()=="1"
end

local articles = {
   {
      priority = 5,
      test = test_hypergates_construction,
      head = N_([[Large Orbital Constructions Spike Curiosity]]),
      body = _([[Throughout the Empire, large orbital constructions have spiked curiosity of passersby, citing difference with standard space station designs. Great Houses have stated they are routine constructions without need of alarm.]]),
   },
   {
      test = test_hypergates_done,
      head = N_([[Hypergate Nausea Syndrome Officially Recognized]]),
      body = _([[The intergalactic association of medical practitioners has officially recognized Hypergate Nausea Syndrome as a separate condition from Hyperspace Nausea Syndrome due to the major differences.]]),
   },
}

return function ()
   return "Generic", nil, nil, articles
end
