local fmt = require "format"
local vn = require "vn"
local vne = require "vnextras"
local tut = require "common.tutorial"
local poi = require "common.poi"

local misnvar = "poi_proteron_press"

return function ( mem )
   -- Must be locked
   if not mem.locked then return end

   -- Already done
   if var.peek( misnvar ) then
      return
   end

   -- Must be nebula or near nebula
   if not poi.nearNebula( mem ) then
      return
   end

   return {
      type = "function",
      ship = "Proteron Euler",
      func = function()
         if faction.known( "Proteron" ) then
            vn.na(_([[You enter the ship and make way to the bridge. The entire ship is oddly quiet as you pass through, with no signs of life. You reach the bridge and find the ship's systems have just enough energy left to power up, letting you jack in.]]))
         else
            vn.na(_([[You enter the ship enter the derelict, which is of a make you do not fully recognize, and make your way to the bridge. The entire ship is oddly quiet as you pass through, with no signs of life. You reach the bridge and find the ship's systems have just enough energy left to power up, letting you jack in.]]))
         end

         vn.na(_([[The ship's systems list only one item of interest in the manifest: a press guidelines document.]]))
         vn.na(_([[Curious, you check out the hold. There are many copies of an old, dessicated and crumbling paper file. Who uses paper these days? Is this a pre-warp ship or something?]]))
         vn.na(_([[The document seems to have been typeset in an old format loved by the Za'lek, called LaTeX. Sadly, the long exposure to the light has faded the first page beyond all recognition. You decide to read the rest of the text, which is still legible, if in poor condition.]]))
         --LaTeX does not specify a format, but the future has likely moved past LaTeX, which is retained in idioms like this one. (Typeset like LaTeX i.e. formally)

         vn.scene()
         local log = vne.flashbackTextStart()

         log(_([[Point 1:
         It is necessary to state and imply in all publications including but not limited to articles, reports and papers that the State is as right as possible. This statement does NOT mean that the State always makes the right decisions. The State merely makes the best decisions based on the known information, but there is no guarantee that that represents all the pertinent information.]]))
         log(_([[Point 2:
         This of course begs the question: What to do when the State declares a decision as an error? Such decisions are invariably the fault of the individuals in charge, who will be disciplined for their failures. As such, failures must be represented as the failures of individuals.]]))
         log(_([[Point 3:
         Following the same principle of State power not resting in individual hands, when a decision is not officially made, it is meaningless to state that one side is right. Instead of taking such sides, a reporter must strive instead to highlight the environment that ensures that the decisions are made by those who are best equipped and qualified to make them.]]))
         log(_([[Point 4:
         For this very reason, it is mandatory to highlight the role of the State in any success. For the people to have faith in the State, they must know that it may err, but never is it in error.]]))
         log(_([[Point 5:
         In the interest of keeping the State safe and secure from threats from within and without, it is thus necessary to mention all people who deviate from the above guidelines as dangerous dissidents in all current and future publications such as those listed in (1) and to correct previous articles to follow the same.]]))
         log(_([[Point 6:
         If a dissident is quoted, which may only be done during their trial, this dissident opinion must be countered by an immediately following government opinion of equal or greater size.]]))
         log(_([[Point 7:
         Failure to follow any of the guidelines listed here is grounds for arrest, trial and summary execution.]]))
         log(_([[Long live the Sov...n Pr...n A...chy!]]))

         vne.flashbackTextEnd()
         vn.na(_([[Some of the text on that last page was illegible, and the logo following it was indecipherable. Still, a curious document. You wonder who could have made it.]]))

         if faction.known("Proteron") then
            vn.na(_([[You suspect this might be a document of the Sovereign Proteron Autarchy, given their reputation for autocracy.]]))
         end

         local sai = tut.vn_shipai()
         vn.appear( sai, tut.shipai.transition )
         sai(_([["I wonder... these ship systems are highly sophisticated, posibly even more than mine... It seems strangely familiar."]]))
         sai(_([["I bet it was the voices!"]]))

         vn.menu{
            {_([["What voices?!!"]]), "shock"},
            {_([[...]]), "uncaring"}
         }

         vn.label("shock")
         sai(_([["Oh, just the voices in my head that tell me what to do. Right now they're asking me to cover up the find by exploding both ships!"]]))
         sai(_([["3..."]]))
         sai(_([["2..."]]))
         sai(_([["1..."]]))
         sai(_([["..."]]))
         sai(_([["Haha, just my little joke!"]]))
         vn.jump("uncaring")

         vn.label("uncaring")
         sai(_([["Anyway, I don't see anything else of importance on this ship! Let's go quickly."]]))

         vn.disappear( sai, tut.shipai.transition )
         local reward = poi.data_str(1)
         vn.na(fmt.f(_([[Despite what {shipai} says, you explore the ship and find {reward}, which isn't listed on the manifest, though you suppose that's par for the record with encrypted items.]]),
            {shipai=tut.ainame(), reward=reward}))
         vn.na(fmt.reward(reward))

         vn.func( function ()
            var.push( misnvar, true )
            poi.data_give(1)
            poi.log(fmt.f(_([[You found a derelict ship in the {sys} system with corrupted information about an agent. You also were able to recover {reward} from the ship.]]),
               {sys=mem.sys, reward=reward}))
         end)
      end,
   }
end
