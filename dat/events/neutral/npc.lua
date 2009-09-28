
--[[
-- Event for creating random characters in the spaceport bar.
--]]


--[[
--    Entry point for the event.
--]]
function create ()
   global_npc = {}

   create_neutral( rnd.rnd(1,5) )

   -- End event on takeoff.
   hook.takeoff( "enter" )
end
--[[
--    Event is over when player takes off.
--]]
function enter ()
   evt.finish()
end


--[[
-- Creates neutral NPC.
--]]
function create_neutral( n )
   neutral_npcNum = 0

   local i = 0
   while i < n do
      neutral_npc()
      i = i + 1
   end
end


--[[
--    NPC talk function
--]]
function neutral_talk( id )
   local data = global_npc[ id ]

   -- Extract the data
   local name = data["name"]
   local msg  = data["msg"]

   -- Display text
   tk.msg( name, msg )
end


function neutral_genMerchant ()
   local portraits = { "none" }
   local desc = {
      "You see a merchant in a brightly coloured robe.",
      "You see a merchant in a polka-dotted vest."
   }
   local msg = {
      "Merchants rock!",
      "You do know merchantst rock don't you?",
      "Let me tell you that merchants rock!"
   }
   local data = {}

   data[1] = "Merchant"
   data[2] = portraits[ rnd.rnd( 1, #portraits ) ]
   data[3] = desc[ rnd.rnd( 1, #desc ) ]
   data[4] = msg[ rnd.rnd( 1, #msg ) ]
   return data
end


function neutral_genCivilian ()
   local portraits = { "none" }
   local desc = {
      "You see a civilian in an ugly work vest.",
      "You see a civilian in the standard work attire.",
      "You see a civilian."
   }
   local msg = {
      "Merchants suck!",
      "Yeah totally, like I was saying, merchants suck!",
      "You do know merchants suck don't you?"
   }
   local data = {}

   data[1] = "Civilian"
   data[2] = portraits[ rnd.rnd( 1, #portraits ) ]
   data[3] = desc[ rnd.rnd( 1, #desc ) ]
   data[4] = msg[ rnd.rnd( 1, #msg ) ]
   return data
end


--[[
--    Generates a neutral NPC
--]]
function neutral_npc ()
   -- Choose type
   local data
   r = rnd.rnd()
   if r > 0.5 then
      data = neutral_genMerchant()
   else
      data = neutral_genCivilian()
   end

   -- Extract the data
   local name = data[1]
   local portrait = data[2]
   local desc = data[3]
   local func = "neutral_talk"
   local msg  = data [4]

   -- Create the NPC
   local id = evt.npcAdd( func, name, portrait, desc, 10 )

   -- Create new data
   local data = {}
   data["name"] = name
   data["msg"]  = msg

   -- Add to global table
   global_npc[ id ] = data
end


