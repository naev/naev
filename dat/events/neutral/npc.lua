
--[[
-- Event for creating random characters in the spaceport bar.
--]]


--[[
--    Entry point for the event.
--]]

lang = naev.lang()
if lang == 'es' then --not translated atm
else --default english
    civ_name = "Civilian"
    civ_portraits = {
        "jorek",
        "thief1",
        "thief2"
    }
    civ_desc = {
        "A worker on his day off.",
        "Just some drunk.",
        "You see a civilian.",
        "You see a highly drunk civilian.",
        "You have the eery suspicion that the individual you spot is severely intoxicated."
    }
    civ_msg = {}
    civ_msg["Independent"] = {
        "I could make you rich, you know.",
        "I once knew a guy who looked like you.",
        "He passes out.",
        "So, as I was saying, merchants suck.",
        "He tells you his entire life story before demanding you pay his bill."
    }
    civ_msg["Empire"] = {
        "The Emperor really is watching, you know."
    }
    civ_msg["Dvaered"] = {
        "Damn FLF..."
    }
    civ_msg["FLF"] = {
        "Damn Dvaered..."
    }
    civ_msg["Frontier"] = {
        "Those FLF guys are doing the right thing, you know."
    }
    civ_msg["Proteron"] = {
        "I don't know what to say..."
    }
    civ_msg["Za'Lek"] = {
        "Do you want to see my prototype?"
    }
    civ_msg["Sirius"] = {
        "I wish I could see the Sirichana"
    }
    civ_msg["Goddard"] = {
        "Not much to report, huh"
    }
    
    merch_name = "Merchant"
    merch_portraits = {
        "jorek",
        "thief1",
        "thief2"
    }
    merch_desc = {
        "You spot a merchant.",
        "You spot a drunk merchant.",
        "A merchant between missions.",
        "A merchant, he reminds you of someone.",
        "There is sufficient evidence for you to make the educated assumption that the person in question has ingested more than the recommended dose of alcohol."
    }
    merch_msg = {
        "You wanna buy some speedsticks?",
        "I have these crystals, you simply must buy some.",
        "How about you buy me a beer?",
        "Go away.",
        "Do you think I look like a mule?"
    }
    
    misn_msg = {}
    --misn_msg["Shadowrun"] = "I would check out that thing, you know..."
end   


function create ()
    global_npc = {}
    faction = planet.cur():faction()
    reputation = player.getFaction(faction:name())
    civ_msg_fac = civ_msg["Independent"]
    if faction ~= "Independent" then
        for key, value in pairs(civ_msg[faction:name()]) do
            table.insert(civ_msg_fac, value)
        end
    end
    civ_misn_msg = {}
    merch_misn_msg = {}
    for key, value in pairs(misn_msg) do
        if player.misnDone(key) ~= true then
            table.insert(civ_misn_msg, value)
            table.insert(merch_misn_msg, value)
        end
    end 
    

    local n = rnd.rnd(1,5)
   
    local i = 0
    while i < n do
        create_npc()
        i = i + 1
    end
    -- End event on takeoff.
    hook.takeoff( "leave" )
end

--[[
--    Generates an NPC
--]]
function create_npc ()
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
   local func = "npc_talk"
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

--[[
--    NPC talk function
--]]
function npc_talk( id )
   local data = global_npc[ id ]

   -- Extract the data
   local name = data["name"]
   local msg  = data["msg"]

   -- Display text
   tk.msg( name, msg )
end


function neutral_genMerchant ()
   local npcdata = {}
   npcdata[1] = merch_name
   npcdata[2] = merch_portraits[ rnd.rnd( 1, #merch_portraits ) ]
   npcdata[3] = merch_desc[ rnd.rnd( 1, #merch_desc ) ]
   if rnd.rnd( 0, 100 ) < 11 then
       npcdata[4] = merch_misn_msg[ rnd.rnd( 1, #merch_misn_msg ) ]
   else
       npcdata[4] = merch_msg[ rnd.rnd( 1, #merch_msg ) ]
   end
   return npcdata
end


function neutral_genCivilian ()
   local npcdata = {}
   npcdata[1] = civ_name
   npcdata[2] = civ_portraits[ rnd.rnd( 1, #civ_portraits ) ]
   npcdata[3] = civ_desc[ rnd.rnd( 1, #civ_desc ) ]
   if rnd.rnd( 0, 100 ) < reputation then
       npcdata[4] = civ_misn_msg[ rnd.rnd( 1, #civ_misn_msg ) ]
   else
       npcdata[4] = civ_msg_fac[ rnd.rnd( 1, #civ_msg_fac ) ]
   end
   return npcdata
end

--[[
--    Event is over when player takes off.
--]]
function leave ()
    evt.finish()
end