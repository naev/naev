--[[

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 3 as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

--]]

-- Capsule function for tk.msg that disables all key input WHILE the msg is open.
function tkMsg(title, msg, keys)
    naev.keyDisableAll()
    enableBasicKeys()
    tk.msg(title, msg)
    if keys ~= nil then
       enableKeys(keys)
    else
       naev.keyEnableAll()
    end
end


-- Capsule function for enabling the keys passed to it in a table, plus some defaults.
function enableKeys(keys)
    naev.keyDisableAll()
    for _, key in ipairs(keys) do
        naev.keyEnable(key, true)
    end
    enableBasicKeys()
end

-- Capsule function for enabling basic, important keys.
function enableBasicKeys()
    local alwaysEnable = { "speed", "menu", "screenshot", "console" }
    for _, key in ipairs(alwaysEnable) do
        naev.keyEnable(key, true)
    end
end

-- Capsule function for naev.keyGet() that adds a color code to the return string.
function tutGetKey(command)
    return "\027b" .. naev.keyGet(command) .. "\0270"
end
