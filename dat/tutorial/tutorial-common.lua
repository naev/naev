-- Capsule function for tk.msg that disables all key input WHILE the msg is open.
function tkMsg(title, msg, keys)
    naev.keyDisableAll()
    tk.msg(title, msg)
    enableKeys(keys)
end

-- Capsule function for enabling the keys passed to it in a table, and ONLY those keys.
function enableKeys(keys)
    naev.keyDisableAll()
    for _, key in ipairs(keys) do
        naev.keyEnable(key, true)
    end
end

-- Capsule function for naev.getKey() that adds a color code to the return string.
function tutGetKey(command)
    return "\027b" .. naev.keyGet(command) .. "\0270"
end