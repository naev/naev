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
      tag = N_([[Large Orbital Constructions Spike Curiosity]]),
      desc = _([[Throughout the Empire, large orbital constructions have spiked curiosity of passersby, citing difference with standard space station designs. Great Houses have stated they are routine constructions without need of alarm.]]),
   },
   {
      priority = 5,
      test = test_hypergates_done,
      tag = N_([[Hypergate Network Goes Live]]),
      desc = _([[In separate press releases, the Great Houses and Soromid have announced the activation of a hypergate network allowing for intersystem long-range travel. Many people flocked to try the new system, with some reporting severe hyperspace nausea.]]),
   },
}

return function ()
   return "Generic", nil, nil, articles
end
