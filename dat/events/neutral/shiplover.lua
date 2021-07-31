--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Ship Lover Quiz">
 <trigger>land</trigger>
 <chance>100</chance>
 <notes>
  <campaign>Minerva</campaign>
  <provides name="Minerva Station" />
 </notes>
</event>
--]]

--[[
-- Recurring ship lover event.
--]]
local vn = require 'vn'
require "numstring"

shiplover_name    = _("Ship Enthusiast")
shiplover_portrait= "shiplover.webp"
shiplover_image   = "shiplover.webp"
shiplover_desc    = _("You see an individual who is playing with a small ship model.")
shiplover_priority= 5

local function getNUnique( t, n )
   local o = {}
   for k,v in ipairs(t) do
      local found = false
      for i,u in ipairs(o) do
         if u==v then
            found = true
            break
         end
      end
      if not found then
         table.insert( o, v )
         if #o >= n then
            break
         end
      end
   end
   return o
end
local function map( f, t )
   local o = {}
   for k,v in ipairs(t) do
      o[k] = f( v )
   end
   return o
end
local function merge_tables( list )
   local o = {}
   for i,t in ipairs(list) do
      for k,v in ipairs(t) do
         table.insert(o,v)
      end
   end
   return o
end
local function increment_var( varname )
   local n = var.peek(varname) or 0
   n = n + 1
   var.push( varname, n )
end

local standard_ships = {
   "Llama",
   "Schroedinger",
   "Gawain",
   "Koala",
   "Quicksilver",
   "Rhino",
   "Mule",
   "Shark",
   "Hyena",
   "Lancelot",
   "Vendetta",
   "Ancestor",
   "Phalanx",
   "Admonisher",
   "Vigilance",
   "Pacifier",
   "Starbridge",
   "Kestrel",
   "Hawking",
   "Goddard",
}
local sirius_ships = {
  "Sirius Fidelity",
  "Sirius Shaman",
  "Sirius Preacher",
  "Sirius Divinity",
  "Sirius Dogma",
}
local soromid_ships = {
  "Soromid Brigand",
  "Soromid Reaver",
  "Soromid Marauder",
  "Soromid Odium",
  "Soromid Nyx",
  "Soromid Ira",
  "Soromid Arx",
  "Soromid Vox",
}
local dvaered_ships = {
  "Dvaered Vendetta",
  "Dvaered Ancestor",
  "Dvaered Phalanx",
  "Dvaered Vigilance",
  "Dvaered Goddard",
}
local empire_ships = {
  "Empire Shark",
  "Empire Lancelot",
  "Empire Admonisher",
  "Empire Pacifier",
  "Empire Hawking",
  "Empire Peacemaker",
}
local zalek_ships = {
  "Za'lek Sting",
  "Za'lek Demon",
  "Za'lek Mephisto",
  "Za'lek Diablo",
  "Za'lek Hephaestus",
}
local pirate_ships = {
  "Pirate Shark",
  "Pirate Vendetta",
  "Pirate Ancestor",
  "Pirate Phalanx",
  "Pirate Admonisher",
  "Pirate Starbridge",
  "Pirate Rhino",
  "Pirate Kestrel",
}
local faction_ships = merge_tables{
   sirius_ships,
   soromid_ships,
   dvaered_ships,
   empire_ships,
   zalek_ships,
   pirate_ships,
}

function gen_question_ship_class ()
   -- Create question
   local ship_list
   if hard then
      ship_list = merge_tables{ standard_ships, faction_ships }
   else
      ship_list = standard_ships
   end
   local ships = rnd.permutation( ship_list )
   local question = string.format(_([["What class is the #o%s#0 ship?"]]), ships[1])
   local classes  = map( function (s) return ship.get(s):classDisplay() end, ships )
   local options  = getNUnique( classes, 5 )
   local answer   = options[1]
   return { type="ship_class", question=question, options=options, answer=answer }
end

function gen_question ()
   return gen_question_ship_class()
end

function create ()
   if not var.peek("testing") then evt.finish() end
   local whitelist = {
      ["Independent"]= true,
      ["Empire"]     = true,
      ["Dvaered"]    = true,
      ["Sirius"]     = true,
      ["Soromid"]    = true,
      ["Za'lek"]     = true,
      ["Goddard"]    = true,
      ["Frontier"]   = true,
   }
   local sfact = system.cur():faction():nameRaw()
   if not whitelist[ sfact ] then
      evt.finish()
   end

   -- Clear met if didn't play last time
   if var.peek("shiplover_quiz") == nil then
      var.pop("shiplover_met")
   end

   -- Generate the question
   question = gen_question()

   -- TODO better and randomized rewards
   cash_reward = 50e3

   -- Set up NPC and hooks
   evt.npcAdd("approach_shiplover", shiplover_name, shiplover_portrait, shiplover_desc, shiplover_priority )
   hook.takeoff("event_end")
end
function event_end ()
   evt.finish()
end

function approach_shiplover ()
   local remove_npc = false
   local first_meet = not var.peek("shiplover_met")

   vn.clear()
   vn.scene()
   local sl = vn.newCharacter( shiplover_name, { image=shiplover_image } )
   vn.transition()

   -- First meeting
   if first_meet then
      vn.na(_("You approach the individual who is making weird noises, likely imitating space ship noises, while playing with a space ship toy."))
      sl(_([["Why hello there. How do you like my new limited edition Lancelot figurine?"]]))
      vn.menu( function () 
         local opts = {
            { _([["It looks great!"]]), "first_great" },
            { _([["It's awful."]]), "first_awful" },
            { _([["Aren't you too young to be here?"]]), "first_tooyoung" },
         }
         local has_lancelot = false
         local lancelot = ship.get("Lancelot")
         for k,v in ipairs( player.ships() ) do
            if v.ship == lancelot then
               has_lancelot = true
               break
            end
         end
         if has_lancelot then
            table.insert( opts, { _([["You know I own a real one."]]), "first_showoff" } )
         end
         return opts
      end )

      vn.label("first_great")
      sl(_([["I know right! It cost me a lot to find it. It was sold out all over! But it was all worth it in the end."]]))
      vn.jump("first_cont1")

      vn.label("first_awful")
      sl(_([["Phhhsssh. You can't appreciate this beauty. Look at the curves! The shining lights! It's a fully anatomically correct 1/128 scale model! You can even see the pilots through the cockpit."
You look closely and indeed are able to see the pilots. The level of detail is fairly ridiculous.]]))
      vn.jump("first_cont1")

      vn.label("first_tooyoung")
      sl(_([["Aren't you too old to be here? It's not like I'm drinking or anything."
They stick their tongue out at you as they still play with the space ship.]]))
      vn.jump("first_cont1")

      vn.label("first_showoff")
      sl(_([["Really? Is it a Sigma-5 squadron Golden Efreeti model too?! You don't know what that is? Stop messing with me, it's probably just another mass-produced Lancelot without any history behind it. This is a masterpiece!"
The lift up their toy Lancelot. You can barely make out a golden Efreeti etched on the side.]]))
      vn.jump("first_cont1")

      vn.label("first_cont1")
      sl(_([["You know what, let's play a game. I know ALL the facts of all the ships. I'll ask you a question and if you get it right, I'll give you a nice reward. What do you say?"]]))
      vn.func( function ()
         var.push("shiplover_met",true)
      end )
   elseif var.peek( "shiplover_quiz")==nil and var.peek("shiplover_met") then
      -- Met, didn't play quiz and came back
      vn.na(_("You once again approach the ship enthusiast."))
      sl(_([["So you're back. Did you change your mind about playing my ship quiz?"]]))
   else
      vn.na(_("You once again approach the ship enthusiast. They seem to move about a lot."))
      sl(_([["Would you like to play another round of the ship quiz? I think the question I prepared will stump you completely."]]))
   end
   vn.menu{
      { _([["Yes!"]]), "play_yes" },
      { _([["No."]]),  "play_no" },
   }

   vn.label("play_no")
   sl(_([["Oh well, your loss!"]]))
   vn.done()

   vn.label("play_yes")
   if question.type == "ship_class" then
      sl(string.format(_([["Great! So here it goes. Listen carefully."

%s]]), question.question))
   end

   -- Show choices
   vn.menu( function ()
      local opts = {}
      local qopts = rnd.permutation( question.options )
      for k,v in ipairs(qopts) do
         table.insert( opts, { v, v } )
      end
      return opts
   end, function( key )
      if key==question.answer then
         vn.jump("answer_right")
      else
         vn.jump("answer_wrong")
      end
   end )

   vn.label("answer_right")
   vn.func( function ()
      increment_var( "shiplover_quiz" )
      increment_var( "shiplover_quiz_right_"..question.type )
   end )
   vn.sfxBingo()
   sl(_([["That's right! Damn, I thought you wouldn't know this one. Here, take this as a reward for your performance."]]))
   vn.func( function ()
      player.pay( cash_reward, true ) -- Don't trigger hooks
   end )
   vn.na(string.format(_("You have received #g%s#0."), creditstring(cash_reward)))
   vn.jump("remove_npc")

   vn.label("answer_wrong")
   vn.func( function ()
      increment_var( "shiplover_quiz" )
      increment_var( "shiplover_quiz_wrong_"..question.type )
   end )
   -- TODO wrong sound
   sl(string.format(_([[They look smug as they exclaim "Wrong!".
"The correct answer was #g%s#0! Better luck next time."
They take their leave.]]), question.answer))

   vn.label("remove_npc")
   vn.func( function () remove_npc = true end )
   vn.run()

   -- Finish event (removes npc)
   if remove_npc then
      evt.finish()
   end
end
