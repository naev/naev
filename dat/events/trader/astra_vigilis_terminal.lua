--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Astra Vigilis Terminal">
 <location>land</location>
 <chance>100</chance>
 <faction>Traders Society</faction>
 <cond>
   player.misnDone("Join Astra Vigilis")
 </cond>
</event>
--]]
--local trader = require "common.trader"
local fmt = require "format"
local vn = require "vn"

-- TODO use Astra Vigilis logo instead
local TERMINAL = "minerva_terminal"
local DIFFICULTY = "bounty_difficulty"

function create ()
   local difficulty = var.peek(DIFFICULTY)
   if not difficulty then
      var.push( DIFFICULTY, N_("Medium") )
   end

   evt.npcAdd( "approach", _("Astra Vigilis Terminal"), TERMINAL, _("A terminal to access the Astra Vigilis databases.") )
   hook.enter("enter")
end

function enter ()
   evt.finish(false)
end

function approach ()
   local terminal = vn.Character.new( _("Astra Vigilis Terminal"), {
      image = TERMINAL,
   } )

   vn.clear()
   vn.scene()
   vn.newCharacter( terminal )
   vn.transition()

   vn.label("menu_prompt")
   vn.na(_([[You access a secure Astra Vigilis terminal and enter your login information.

What do you want to do?]]))
   vn.label("menu")
   vn.menu{
      --{ _("Special Bounties"), "special" },
      { _("Modify Bounty Difficulty"), "difficulty" },
      { _("Leave."), "leave" },
   }

   vn.label("difficulty")
   vn.na( function ()
      -- TODO don't have them all available at the start
      return fmt.f(_([[Current bounty difficulty is {difficulty}. Rewards scale with difficulty. What difficulty do you want your bounties?

{list}]]), {
      list = _([[1. Easy
2. Medium
3. Hard
4. Challenging
5. Extreme]]),
      difficulty = "#b".._(var.peek("bounty_difficulty")).."#0",
   } )
   end )
   vn.menu( function ()
      local opts = {
         { _("Nevermind."), "menu_prompt" },
      }
      local cur = var.peek("bounty_difficulty")
      for k,v in ipairs{
         N_("Extreme"),
         N_("Challenging"),
         N_("Hard"),
         N_("Medium"),
         N_("Easy"),
      } do
         if v ~= cur then
            table.insert( opts, 1, {v, {k,v}} )
         end
      end
      return opts
   end, function ( opt )
      if type(opt)==string then
         return vn.jump(opt)
      end
      var.push( DIFFICULTY, opt[2] )
      vn.jump("difficulty_changed")
   end)

   vn.label("difficulty_changed")
   vn.na( function ()
      local cur = var.peek("bounty_difficulty")
      return fmt.f(_([[Bounty difficulty set to {difficulty}.

What do you want to do?]]), {
         difficulty = "#b"..cur.."#0",
      } )
   end )
   vn.jump("menu")

   vn.label("leave")
   vn.run()
end
