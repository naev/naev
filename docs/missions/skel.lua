-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
end

		
--[[ Create the mission - OBLIGATORY
-- Computer - do not accept, set the strings for reward/desc/title which
--    are what will appear at the planet mission computer
-- Bar - do accept, this is what'll be run when the player enters the bar,
--    so whatever doesn't get accepted will be trashed
--]]
function create()
end

--[[ Accepts the mission - OBLIGATORY for mission computer, USELESS elsewhere
-- Mission is accepted, only useful at the mission computer, otherwise it'll
-- never get called unless you call it from another function
--]]
function accept()
end

