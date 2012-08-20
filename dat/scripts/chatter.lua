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
