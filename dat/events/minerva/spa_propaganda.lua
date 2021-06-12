--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Spa Propaganda">
 <trigger>none</trigger>
 <chance>0</chance>
 <flags>
  <unique />
 </flags>
 <notes>
  <campaign>Minerva</campaign>
  <requires name="Spa Propaganda" />
  <done_evt name="Minerva Station Altercation 2">Random Event</done_evt>
 </notes>
</event>
--]]

--[[
-- Triggered from station.lua
--]]

local minerva = require 'campaigns.minerva'
local vn = require 'vn'

terminal = minerva.terminal

function create()
   vn.clear()
   vn.scene()
   local t = vn.newCharacter( minerva.vn_terminal() )
   vn.music( minerva.loops.news )
   vn.transition()
   vn.na(_("You are minding your own business when suddenly a big fanfare plays while the lights begin strobing."))
   t(_([[The terminals all turn on in union and start blasting out their loudspeakers.
"WELCOME LADIES AND GENTLEMEN. ARE YOU HAVING A GOOD TIME?"
Some of the patrons cheer at the terminals.]]))
   t(_([["MINERVA STATION IS PROUD TO ANNOUNCE A VERY SPECIAL OPPORTUNITY FOR EVERYONE. TO CELEBRATE THE FIRST CYCLE SINCE THE INAUGURATION OF THE LUXURIOUS MINERVA STATION NATURAL HOT SPRINGS AND THERMAL SPA, WE WOULD LIKE TO GIVE EVERYONE AN OPPORTUNITY TO BE ABLE TO ENJOY THE HEALING AND LIFE-EXTENDING PROPERTIES OF THE NEBULA-DUST INFUSED MINERAL WATER."]]))
   t(_([["IN TRADITIONAL MINERVA FASHION, WE WILL DO A LOTTERY WHERE EVERYONE WHO BUYS A TICKET WILL BE ENTERED TO SPEND AN UNFORGETTABLE NIGHT AT THE SPA, THE FAVOURITE OF THE EMPEROR'S THIRD PET IGUANA'S CARETAKER."]]))
   t(_([["AND IF YOU THINK THAT IS ALREADY EXCITING ENOUGH, PREPARE TO DROP YOUR JAW IN DISBELIEF! NOT ONLY WILL YOU BE ABLE TO ENJOY THE SPA, WE HAVE A VERY SPECIAL SURPRISE FOR YOU!"
The terminal pauses for effect.]]))
   t(_([["YOU WILL BE ABLE TO SPEND YOUR TIME AT THE SPA WITH OUR ONE AND ONLY CYBORG CHICKEN, THE BELOVED MASCOT OF MINERVA STATION, AND 3 TIME BACK-TO-BACK WINNER OF THE 'I WOULD EAT THIS IF IT WEREN'T ILLEGAL' POPULAR HOLO-CAST SHOW."]]))
   t(_([["EVEN IF YOU HAVE BEEN TO THE SPA BEFORE, THIS IS A ONCE IN A LIFETIME OPPORTUNITY THAT MANY WOULD DIE FOR. I KNOW I WOULD."]]))
   t(_([["TO PARTICIPATE, ALL YOU HAVE TO DO IS BUY A TICKET AT ANY MINERVA STATION TERMINAL. PARTICIPATION COST IS #p100 MINERVA TOKENS#0. ONLY ONE TICKET PER PARTICIPANT. SOME TERMS AND CONDITIONS MAY APPLY."]]))
   t(_([["AND REMEMBER, LIFE IS SHORT, SPEND IT AT MINERVA STATION."]]))
   vn.na(_("As if nothing happened, Minerva Station returns to normality."))
   vn.run()

   -- One off event
   evt.finish(true)
end
