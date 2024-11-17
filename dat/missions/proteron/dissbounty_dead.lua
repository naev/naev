--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Proteron Dissident Dead Or Alive Bounty">
 <priority>4</priority>
 <cond>
   require("misn_test").mercenary()
 </cond>
 <chance>360</chance>
 <location>Computer</location>
 <faction>Proteron</faction>
 <notes>
  <tier>3</tier>
 </notes>
</mission>
--]]
--[[

   Dead or Alive Proteron Dissident Bounty

--]]
local fmt = require "format"
local lmisn = require "lmisn"
local bounty = require "common.bounty"

local subdue_text = {
   _("You and your crew infiltrate the ship's pathetic security and subdue the dissident. You transport them to your ship."),
   _("Your crew has a difficult time getting past the ship's security, but eventually succeeds and subdues the dissident."),
   _("The ship's security system turns out to be no match for your crew. You infiltrate the ship and capture the dissident."),
   _("Your crew infiltrates the ship and captures the dissident."),
   _("Getting past this ship's security was surprisingly easy. Didn't this dissident know that they were wanted?"),
}

local pay_kill_text = {
   _("After verifying that you killed your target, an officer hands you your pay."),
   _("After verifying that the target is indeed dead, the tired-looking officer smiles and hands you your pay."),
   _("The officer thanks you and promptly hands you your pay."),
   _("The officer takes you into a locked room, where the death of your target is quietly verified. The officer then pays you and sends you off."),
   _("The officer verifies the death of your target, goes through the necessary paperwork, and hands you your pay, looking bored the entire time."),
}

local pay_capture_text = {
   _("An officer takes the dissident into custody and hands you your pay."),
   _("The dissident puts up a fight as you take them to the officer, who then hands you your pay."),
   _("The officer you deal with seems to especially dislike dissidents. The one you captured is taken off your hands and you are handed your pay without a word."),
   _("An officer rushes the fearful-looking dissident into a secure hold, pays you the appropriate bounty, and takes the dissident into custody."),
   _("The officer you greet gives you a puzzled look when you say that you captured your target alive. Nonetheless, they politely take the dissident off of your hands and hand you your pay."),
}

local share_text = {
   _([["Greetings. I can see that you were trying to collect a bounty. Well, as you can see, I earned the bounty, but I don't think I would have succeeded without your help, so I've transferred a portion of the bounty into your account."]]),
   _([["Sorry about getting in the way of your bounty. I don't really care too much about the money, but I just wanted to make sure the galaxy would be rid of that scum. So as an apology, I would like to offer you the portion of the bounty you clearly earned. The money will be in your account shortly."]]),
   _([["Hey, thanks for the help back there. I don't know if I would have been able to handle that dissident alone! Anyway, since you were such a big help, I have transferred what I think is your fair share of the bounty to your bank account."]]),
   _([["Heh, thanks! I think I would have been able to take out the target by myself, but still, I appreciate your assistance. Here, I'll transfer some of the bounty to you, as a token of my appreciation."]]),
   _([["Ha ha ha, looks like I beat you to it this time, eh? Well, I don't do this often, but here, have some of the bounty. I think you deserve it."]]),
}

-- Messages
local msg = {
   _("Your target got away."),
   _("Another pilot eliminated your target."),
   _("You have left the {sys} system."),
}

mem.osd_title = _("Bounty Hunt")
mem.osd_msg = {
   _("Fly to the {sys} system"),
   _("Kill or capture your target"),
   _("Land in {fct} territory to collect your bounty"),
}

-- luacheck: globals get_faction
function get_faction()
   return faction.dynAdd( "Independent", "Proteron Dissident", _("Proteron Dissident"), {ai="proteron_dissident"} )
end

function create ()
   local payingfaction = faction.get("Proteron")

   local systems = lmisn.getSysAtDistance( system.cur(), 1, 3,
      function(s)
         local p = s:presences()["Proteron Dissident"]
         return p ~= nil and p > 0
      end )

   if #systems == 0 then
      -- No dissidents nearby
      misn.finish( false )
   end

   local missys = systems[ rnd.rnd( 1, #systems ) ]
   if not misn.claim( missys ) then misn.finish( false ) end

   local pname = _("Target Dissident")
   local choices = { "Schroedinger", "Hyena", "Llama", "Gawain" }
   local pship = choices[ rnd.rnd( 1, #choices ) ]
   local reward = 250e3 + rnd.sigma() * 50e3
   local reputation = rnd.rnd( 1, 2 )

   -- Set mission details
   misn.setTitle( fmt.f( _("PD: Dead or Alive Bounty in {sys}"), {sys=missys} ) )
   misn.setDesc( fmt.f( _("A political dissident was recently seen in the {sys} system. {fct} authorities want this dissident dead or alive. The dissident may disappear you take too long to reach the {sys} system."), {sys=missys, fct=payingfaction} ) )
   misn.setReward( mem.credits )

   bounty.init( missys, pname, pship, nil, reward, {
      payingfaction     = payingfaction,
      reputation        = reputation,
      targetfactionfunc = "get_faction", -- have to pass by name
      msg_subdue        = subdue_text,
      msg_killed        = pay_kill_text,
      msg_captured      = pay_capture_text,
      msg_shared        = share_text,
      msg_leftsystem    = msg,
      alive_only        = false,
      osd_objective     = _("Capture {plt}"),
   } )
end

function accept ()
   misn.accept()
   bounty.accept()
end
