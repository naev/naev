-- Enumerates the arguments passed to it. Arguments are used as keys and will be assigned numbers in the order they are passed.
-- 
-- Example usage: my_enum = enumerate({"first", "second", "third"})
-- Example usage: print(my_enum.first)
function enumerate(list)
    local enum = {}
    for i, j in ipairs(list) do
        enum[j] = i
    end
    return enum
end
