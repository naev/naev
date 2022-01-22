package.path = package.path .. ";lua-repl/?.lua;lua-repl/?/init.lua"

local repl = require "repl.console"

io.stderr = {} -- luacheck: ignore
function io.stderr:write(str) printRaw(str) end -- luacheck: ignore

repl:loadplugin 'linenoise'
repl:loadplugin 'history'
repl:loadplugin 'completion'
repl:loadplugin 'autoreturn'
repl:loadplugin 'pretty_print'
repl:loadplugin 'semicolon_suppress_output'

return coroutine.create(function() repl:run() end)
