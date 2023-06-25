--local fmt = require "format"
local portrait = require "portrait"
local npc = require "common.npc"
local pir = require "common.pirate"

-- State. Nothing persists.
local msg_combined

local desc_list = {}
desc_list["generic"] = {
   _("A pirate idly drinking some grog."),
   _("An angry looking pirate with a drink."),
   _("You seen an imposing pirate."),
   _("A pirate who may have had too much grog."),
   _("A careless pirate spilling their drink all over."),
   _("A pirate relaxing after a long day of pillaging."),
   _("You see a pirate chilling at the bar."),
   _("A jovial looking pirate."),
   _("A pirate that smells weirdly of fish."),
   _("A tattooed space buccaneer, lazily flipping a coin."),
   _("A weathered space pirate keeping a watchful eye on the room."),
   _("A grizzled pirate nursing a mug of grog."),
   _("A lone pirate silently sipping a drink in the corner."),
   _("A pirate with a menacing grin, scanning the room for trouble."),
   _("A seasoned pirate studying a star chart."),
   _("A swashbuckling space pirate, sipping on a neon-colored cocktail."),
   _("A veteran space pirate, leaning back into their worn-out chair, puffing on a cigar."),
   _("A suave-looking well-groomed pirate sipping some finely aged space rum."),
}

local msg_lore = {
   _([["Hi mate. Money or your life! Heh heh, just messing with you."]]),
   _([["Hey, look at these new scars I got!"]]),
   _([["Have you heard of the Pirates' Code? They're more guidelines than rules…"]]),
   _([["My gran once said to me, 'Never trust a pirate.' Well, she was right! I got a pretty credit chip outta her wallet last time I saw her, and I'd do it again."]]),
   _([["I don't understand why some pirates talk like 16th-century Earth pirates even though that planet is literally dead."]]),
   _([["I may be a pirate who blows up ships and steals for a living, but that inner nebula still kind of freaks me out."]]),
   _([["Damn Empire stopped my heist a few decaperiods ago. Just wait'll they see me again!"]]),
   _([["There's a pirate clanworld I really wanted to get to, but they wouldn't let me in because I'm a 'small-time pirate'! Sometimes I think I'll never make it in this line of work…"]]),
   _([["I was around before Haven was destroyed, you know! Funny times. All the pirates were panicking and the Empire was cheering thinking that we were done for. Ha! As if! It barely even made a difference. We just relocated to New Haven and resumed business as usual."]]),
   _([["Back when I started out in this business all you could do was go around delivering packages for other people. Becoming a pirate was real hard back then, but I got so bored I spent several decaperiods doing it. Nowadays things are way more exciting for normies, but I don't regret my choice one bit!"]]),
   _([["Flying a real big ship is impressive, but it's still no pirate ship. I mean, I respect ya more if you're flying a Goddard than if you're flying a civilian Lancelot, but the best pirates fly the good old Pirate Kestrel!"]]),
   _([["I swear I saw a bunch of cats in a trench coat pretending to be a pirate once. Must have been a weird dream. I should quite drinking…"]]),
   _([["There are four major pirate clans: the Wild Ones, the Ravens, the Dreamers, and the Black Lotus. It's sometimes hard for them to get along."]]),
   _([["Pirate assemblies are the pinnacle of pirate society. Booze! Grog! Fights! Shanties! Jigs! More Grog! It's great fun, just don't drink as much to forget it all!"]]),
   -- This is the 'space pirate shanty' (by theElerd)
--[[
I lost my friend and fam’ly to a pirate raid
The captain stood before me, I thought meself for dead
Instead he told me rise up, come to the endless sea
Into the black I followed, a pirate’s life for me.

Yo ho yo ho a pirate life for me
In endless black we gonna attack
A pirate’s life for me

Yo ho yo ho, up in the endless sea
I may be hanged, don’t give a damn
A pirate’s life for me

The empire sent their warships, to capture our fair crew
A thousand ships before us, and surely we were screwed
The captain said “Me hearties! At last we sail to hell!”
My life was maybe short, but I’m sure I’ve spent it well

Yo ho yo ho go down on your knee
Man the guns and hit them hard
A pirate’s death for me

Yo ho yo ho life with sword and gun
May get you wealth, just fly in stealth
But sure you won’t see home
--]]
   _([["Yo ho yo ho a pirate life for me
In endless black we gonna attack
A pirate’s life for me

Yo ho yo ho, up in the endless sea
I may be hanged, don’t give a damn
A pirate’s life for me"]]),
   _([["I love the space pirate shanty, but every time I learn the lyrics I end up getting piss drunk and forget them the next morning…"]]),
}

local msg_tip = {
   _([["To infiltrate a naval base, I once switched into a Koala to pretend to be a deliveryperson. To my surprise, other space pirates and marauders didn't realize who I was and shot at me! From now on I stick to Skull & Bones ships."]]),
   _([["Y'know, I got into this business by accident to tell the truth. But what can you do? I could get a fake transponder and pretend to be someone else but I'd get caught eventually and I'd lose my fame as a pirate."]]),
   _([["One of my favourite things to do is buy a fake transponder and then deliver as much contraband as I can before I get caught. It's great fun, and finding out that my identity's been discovered gives me a rush!"]]),
   _([["Just caught an old mate ferrying tourists for credits. Nearly puked out my grog! Your reputation won't survive for long working for our victims."]]),
   _([["There is nothing more off-putting to other pirates than normal work. It's the best way to tank your reputation with all pirates!"]]),
   _([["Marauders don't list much to other pirates. If it's too much of a hassle to blast them to bits, you can always pay them to go away."]]),
   _([["Stealth works best when you have a ship equipped for the job. Try to keep your mass low and use stealth improving outfits."]]),
   _([["If you get caught with contraband, you can try to bribe your way out of it. Credits talk!"]]),
}

local fct_wildones = faction.get("Wild Ones")
local fct_ravenclan = faction.get("Raven Clan")
local fct_dreamerclan = faction.get("Dreamer Clan")
local fct_blacklotus = faction.get("Black Lotus")
local function test_wildones () return spob.cur():faction()==fct_wildones end
local function test_ravenclan () return spob.cur():faction()==fct_ravenclan end
local function test_dreamerclan () return spob.cur():faction()==fct_dreamerclan end
local function test_blacklotus () return spob.cur():faction()==fct_blacklotus end
local function test_chapter0 () return (player.chapter()=="0") end
local function tneg( f ) return function () return not f() end end

local msg_cond = {
   { tneg(test_wildones), _([["The Wild Ones clan is too out of control for me. It seems like all they want to do is pillage and destroy!"]]) },
   { tneg(test_blacklotus), _([["The Black Lotus pirates are so boring. All they do is paperwork and extortion. Can you call yourself a pirate if you don't pillage?"]]) },
   { tneg(test_dreamerclan),_([["Dreamer Clan pirates never show up to the pirate assemblies. Probably doing too many illegal substances…"]]) },
   { test_ravenclan,_([["If it weren't for the Raven Clan pirates, piracy would have gone extinct ages ago! Our supplies and organization are second to none!"]]) },
   { test_dreamerclan,_([["I ate a weird mushroom yesterday, and I haven't stopped tripping. Are you real?"]]) },
   { test_chapter0,_([["Ya noticed all the large constructions being built recently in space? They need a lot of rare resources which the resource ships great targets for plundering! Arr!"]]) },
   { tneg(test_chapter0), _([["There been news of all these hypergates going online throughout the Empire. Rumour's that the Black Lotus have one too!"]]) },
}

-- Returns a lore message for the given faction.
local function getMessageLore ()
   return msg_lore[ rnd.rnd(1,#msg_lore) ]
end

local function getMessage( lst )
   if #lst == 0 then
      return getMessageLore()
   end
   return lst[ rnd.rnd(1, #lst) ]
end

return function ()
   local cur, _scur = spob.cur()
   local tags = cur:tags()

   -- Need presence
   if not pir.factionIsPirate( cur:faction () ) then
      return nil
   end

   -- Create a list of conditional messages
   msg_combined = npc.combine_cond( msg_cond )

   -- Add tag-appropriate descriptions
   local descriptions = npc.combine_desc( desc_list, tags )

   local function gen_npc()
      local name = _("Pirate")
      local desc = descriptions[ rnd.rnd(1,#descriptions) ]
      local prt  = portrait.get( "Pirate" )
      local image = portrait.getFullPath( prt )
      local msg
      local r = rnd.rnd()
      if r <= 0.45 then
         msg = getMessageLore()
      elseif r <= 0.7 then
         msg = getMessage( msg_tip )
      else
         msg = getMessage( msg_combined )
      end
      return { name=name, desc=desc, portrait=prt, image=image, msg=msg }
   end

   return { create=gen_npc }
end
