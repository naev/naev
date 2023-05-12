--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Ship Lover Quiz">
 <location>land</location>
 <chance>10</chance>
 <notes>
  <tier>1</tier>
 </notes>
</event>
--]]

--[[
-- Recurring ship lover event.
--]]
local vn = require 'vn'
local lg = require 'love.graphics'
local fmt = require "format"

local shiplover_portrait= "shiplover.webp"
local shiplover_image   = "shiplover.webp"
local shiplover_priority= 5

local question_data, reward -- Non-persistent state

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

local faction_tags = { "sirius", "soromid", "dvaered", "empire", "zalek", "pirate" }
local all_tags = merge_tables{ faction_tags, {"standard"} }
local tagged_ships = {}
for i, ship in ipairs(ship.getAll()) do
   local tags = ship:tags()
   if not tags["noplayer"] then
      for j, tag in ipairs(all_tags) do
         if tags[tag] then
            tagged_ships[tag] = tagged_ships[tag] or {}
            table.insert( tagged_ships[tag], ship:nameRaw() )
         end
      end
   end
end

local standard_ships = tagged_ships.standard
local faction_list = {}
for i, tag in ipairs(faction_tags) do
   table.insert( faction_list, tagged_ships[tag] )
end
local faction_ships = merge_tables( faction_list )

local function gen_question_ship_class( hard )
   -- Create question
   local ship_list
   if hard then
      ship_list = merge_tables{ standard_ships, faction_ships }
   else
      ship_list = standard_ships
   end
   local ships = rnd.permutation( ship_list )
   local question = fmt.f(_([["What class is the #o{ship}#0 ship?"]]), {ship=_(ships[1])})
   local classes  = map( function (s) return ship.get(s):classDisplay() end, ships )
   local options  = getNUnique( classes, 5 )
   local answer   = options[1]
   return { type="ship_class", question=question, options=options, answer=answer }
end

local function gen_question_ship_guess( hard )
   -- Create question
   local ship_list
   if hard then
      ship_list = faction_list[ rnd.rnd(1,#faction_list) ]
   else
      ship_list = standard_ships
   end
   local ships = rnd.permutation( ship_list )
   local question = _([["What is the name of this ship?"]])
   local options  = getNUnique( ships, 5 )
   local answer   = options[1]
   return { type="ship_guess", question=question, options=options, answer=answer }
end

local function gen_question( difficulty )
   local r = rnd.rnd()
   if difficulty == 1 then
      if r < 0.6 then
         return gen_question_ship_guess( false )
      else
         return gen_question_ship_class( false )
      end
   --elseif difficulty == 2 then
   else
      if r < 0.5 then
         return gen_question_ship_class( true )
      else
         return gen_question_ship_guess( true )
      end
   end
end

local function shiplover_log( msg )
   -- Create the log if necessary.
   shiplog.create( "shiplover", _("Ship Quiz"), _("Neutral") )
   shiplog.append( "shiplover", msg )
end

local nplayed
function create ()
   local pnt = spob.cur()

   -- Ignore claimed systems (don't want to ruin the atmosphere)
   if not naev.claimTest( system.cur() ) then evt.finish() end

   -- Do not spawn on restricted spobs
   if pnt:tags().restricted then evt.finish() end

   -- Ignore on uninhabited and planets without bars
   local services = pnt:services()
   local flags = pnt:flags()
   if not services.inhabited or not services.bar or flags.nomissionspawn then
      evt.finish()
   end

   -- Only available on whitelisted factions
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

   local pfact = pnt:faction()
   if not pfact or not whitelist[ pfact:nameRaw() ] then
      evt.finish()
   end

   -- Make sure not same system as last time
   local lastplanet = var.peek("shiplover_lastplanet")
   if lastplanet then
      if spob.get(lastplanet):system() == system.cur() then
         evt.finish()
      end
   end

   -- Only allow once every 10 periods at best
   local lastplayed = var.peek("shiplover_lastplayed")
   if not lastplayed then
      lastplayed = time.get():tonumber()
      var.push( "shiplover_lastplayed", lastplayed )
   end
   lastplayed = time.fromnumber( lastplayed )
   if lastplayed + time.new( 0, 10, 0 ) > time.get() then
      evt.finish()
   end

   -- See how many times cleared
   local quiz_types = {
      "ship_class",
      "ship_guess",
   }
   local nwon = 0
   local nlost = 0
   for k,v in ipairs(quiz_types) do
      local nw = var.peek( "shiplover_quiz_right_"..v ) or 0
      local nl = var.peek( "shiplover_quiz_wrong_"..v ) or 0
      nwon = nwon + nw
      nlost = nlost + nl
   end
   nplayed = nwon + nlost

   -- Clear met if didn't play last time
   if nplayed == 0 then
      var.pop("shiplover_met")
   end

   -- Increase the difficulty as progresses
   local difficulty = 1
   local cash_reward = 10e3
   if nwon >= 5 then
      difficulty = 2
      cash_reward = 25e3
   elseif nwon > 10 then
      difficulty = 3
      cash_reward = 50e3
   end

   -- Generate the question
   question_data = gen_question( difficulty )

   -- Determine reward
   reward = {}
   if nwon == 4 and player.numOutfit("Trading Card (Common)")<1 then
      local outfit_reward = outfit.get("Trading Card (Common)")
      reward.func = function ()
         shiplover_log(fmt.f(_("You obtained 1 {outfit} from the Ship Enthusiast for getting 5 quizzes right."), {outfit=outfit_reward}))
         player.outfitAdd( outfit_reward )
      end
      reward.msg_shiplover = _([["Wow. This is the 5th time you got my quiz right. This deserves a special reward. Here, take this special trading card. Don't worry, I have a dozen like it. I'll have to step up my quiz game from now on."]])
      reward.msg_obtain = fmt.reward(outfit_reward)

   elseif nwon == 9 and player.numOutfit("Trading Card (Uncommon)")<1 then
      local outfit_reward = outfit.get("Trading Card (Uncommon)")
      reward.func = function ()
         shiplover_log(fmt.f(_("You obtained 1 {outfit} from the Ship Enthusiast for getting 10 quizzes right."), {outfit=outfit_reward}))
         player.outfitAdd( outfit_reward )
      end
      reward.msg_shiplover = _([["Wow. This is the 10th time you got my quiz right. You are doing much better than I anticipated. Here, take one of my favourite trading cards. Make sure not to lose it, this one is fairly special! I'll have to think of better quizzes from now on."]])
      reward.msg_obtain = fmt.reward(outfit_reward)

   elseif nwon == 24 and player.numOutfit("Trading Card (Rare)")<1 then
      local outfit_reward = outfit.get("Trading Card (Rare)")
      reward.func = function ()
         shiplover_log(fmt.f(_("You obtained a {outfit} from the Ship Enthusiast for getting 25 quizzes right."), {outfit=outfit_reward}))
         player.outfitAdd( outfit_reward )
      end
      reward.msg_shiplover = _([["Damn. This is the 25th time you got my quiz right. Nobody has played my quiz with me for this long. I guess I have to commemorate this in a special way. Here, take one of the rarest cards in my collection. I only have one copy of this one so make sure to take good care of it. No! Don't take it out of the card foil! It might get damaged that way!"]])
      reward.msg_obtain = fmt.reward(outfit_reward)

   else
      reward.func = function ()
         shiplover_log(fmt.f(_("You obtained {credits} from the Ship Enthusiast for getting a quiz right."), {credits=fmt.credits(cash_reward)}))
         player.pay( cash_reward, true ) -- Don't trigger hooks
      end
      reward.msg_shiplover = fmt.f(n_(
         [["That's right! Damn, I thought you wouldn't know this one. You've solved my quiz {n} time now! Here, take this as a reward for your performance."]],
         [["That's right! Damn, I thought you wouldn't know this one. You've solved my quiz {n} times now! Here, take this as a reward for your performance."]],
         nwon+1), {n=nwon+1})
      reward.msg_obtain = fmt.reward(cash_reward)
   end

   -- Set up NPC and hooks
   evt.npcAdd("approach_shiplover", _("Ship Enthusiast"), shiplover_portrait, _("You see a surprisingly young individual who is playing with a small ship model."), shiplover_priority )
   hook.takeoff("event_end")
end
function event_end ()
   evt.finish()
end

function approach_shiplover ()
   local remove_npc = false
   local restore_vn
   local first_meet = not var.peek("shiplover_met")

   vn.reset()
   vn.scene()
   local sl = vn.newCharacter( _("Ship Enthusiast"), { image=shiplover_image, flip=false} )
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
They lift up their toy Lancelot. You can barely make out a golden Efreeti etched on the side.]]))
      vn.jump("first_cont1")

      vn.label("first_cont1")
      sl(_([["You know what, let's play a game. I know ALL the facts of all the ships. I'll ask you a question and if you get it right, I'll give you a nice reward. What do you say?"]]))
      vn.func( function ()
         var.push("shiplover_met",true)
      end )
   elseif nplayed==0 and var.peek("shiplover_met") then
      -- Met, didn't play quiz and came back
      vn.na(_("You once again approach the ship enthusiast."))
      sl(_([["So you're back. Did you change your mind about playing my ship quiz?"]]))
   else
      vn.na(_("You once again approach the ship enthusiast. They seem to move about a lot."))
      sl(_([["Would you like to play another round of the ship quiz? I think the question I prepared will stump you completely!"]]))
   end
   vn.menu{
      { _([["Yes!"]]), "play_yes" },
      { _([["No."]]),  "play_no" },
   }

   vn.label("play_no")
   sl(_([["Oh well, your loss!"]]))
   vn.done()

   vn.label("play_yes")
   if question_data.type == "ship_class" then
      sl(_([["Great! So here it goes. Listen carefully."]]) .. "\n\n" .. question_data.question)
   elseif question_data.type == "ship_guess" then
      local nw, _nh = naev.gfx.dim()
      local shipgfx = lg.newImage( ship.get(question_data.answer):gfxComm() )
      local shipchar = vn.Character.new( "ship", {image=shipgfx, pos="left"} )
      local slpos, slnewpos
      local function runinit ()
         slpos = sl.offset
         slnewpos = 0.75
      end
      vn.animation( 1, function( alpha, _dt, _params )
         sl.offset = slnewpos*alpha + slpos*(1-alpha)
      end, nil, "ease-in-out", runinit )
      vn.appear( shipchar )
      vn.func( function ()
         vn.menu_x = math.min( -1, 500 - nw/2 )
      end )
      sl(_([["Great! So take a look at this ship and listen carefully."]]) .. "\n\n" .. question_data.question)
      function restore_vn ()
         vn.disappear( shipchar )
         vn.animation( 1, function( alpha, _dt, _params )
            sl.offset = slpos*alpha + slnewpos*(1-alpha)
         end, nil, "ease-in-out" )
      end
   end

   -- Show choices
   vn.menu( function ()
      local opts = {}
      local qopts = rnd.permutation( question_data.options )
      for k,v in ipairs(qopts) do
         table.insert( opts, { _(v), v } )
      end
      return opts
   end, function( key )
      if key==question_data.answer then
         vn.jump("answer_right")
      else
         vn.jump("answer_wrong")
      end
   end )

   vn.label("answer_right")
   if restore_vn then restore_vn() end
   vn.func( function ()
      increment_var( "shiplover_quiz_right_"..question_data.type )
   end )
   vn.sfxBingo()
   -- Give reward
   sl( reward.msg_shiplover )
   vn.func( function () reward.func() end )
   vn.na( reward.msg_obtain )
   vn.jump("remove_npc")

   vn.label("answer_wrong")
   if restore_vn then restore_vn() end
   vn.func( function ()
      shiplover_log(_("You got the Ship Enthusiast's quiz wrong."))
      increment_var( "shiplover_quiz_wrong_"..question_data.type )
   end )
   -- TODO wrong sound
   sl(fmt.f(_([[They look smug as they exclaim "Wrong!".
"The correct answer was #g{answer}#0! Better luck next time."
They take their leave.]]), {answer=_(question_data.answer)}))

   vn.label("remove_npc")
   vn.func( function () remove_npc = true end )
   vn.run()

   -- Finish event (removes npc)
   if remove_npc then
      var.push( "shiplover_lastplayed", time.get():tonumber() )
      var.push( "shiplover_lastplanet", spob.cur():nameRaw() )
      evt.finish()
   end
end
