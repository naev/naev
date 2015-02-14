--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]

-- Make a pilot say a line, if he is alive. Mainly useful in sequential chat messages.
-- argument chat: A table containing:
-- pilot: The pilot to say the text
-- text: The text to be said
-- 
-- Example usage: hook.timer(2000, "chatter", {pilot = p, text = "Hello, space!"})
function chatter(chat)
    if chat.pilot:exists() then
        chat.pilot:comm(chat.text)
    end
end
