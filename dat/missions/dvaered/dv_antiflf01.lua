--[[
-- This is the first mission in the anti-FLF Dvaered campaign
--]]

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    title = {}
    text = {}
    title[1] = ""
    text[1] = [["Your arrival is timely, citizen," the Dvaered commanding officer tells you. "Listen up. We were in a firefight with a rogue terrorist operative, but the bastard knocked out our engines and most of our primary systems before we could nail him. Fortunately, I think we inflicted serious damage on him as well, so he should still be around here somewhere. My sensors are down, though, so I can't tell for certain."
    The officer draws himself up and assumes the talking-to-subordinates tone that is so typical for Dvaered commanders. "Citizen! You are hereby charged to scout the area, dispose of the enemy ship, then deliver me and my crew to the nearest Dvaered controlled system!"]]
   
end

function create()
    misn.accept()
    
end

function abort()
    misn.finish(false)
end

misn.finish(false) -- deleteme