--[[---
Command-line library. See https://github.com/hoelzro/lua-linenoise.

@module linenoise
--]]

local ln = {}

--[[---
Return a line of user input (generally after yielding to Naev's event loop), or nil if no more is available.

TODO This is a stub implementation that doesn't work.

   @tparam string prompt A prompt like "> " or ">> " to put in front of the line.
--]]
function ln.linenoise( prompt )
   return coroutine.yield( prompt )
end

--[[---
Adds a line to the history list.

TODO This is a stub implementation that doesn't work.

   @tparam string line Input line to add.
--]]
function ln.historyadd( line )
   local _line = line
end

--[[---
Saves the history to the given file path.

TODO This is a stub implementation that doesn't work.

   @tparam string filename PhysicsFS path to the history file.
--]]
function ln.historysave( filename )
   local _filename = filename
end

--[[---
Loads the history from the given file path.

TODO This is a stub implementation that doesn't work.

   @tparam string filename PhysicsFS path to the history file.
--]]
function ln.historyload( filename )
   local _filename = filename
end

--[[---
Sets the completion callback (which should call ln.addcompletion() once per suggested completion of "line").

TODO This is a stub implementation that doesn't work.

   @tparam function callback function(completions, line) which may call ln.addcompletion() with the opaque completions object.
--]]
function ln.setcompletion( callback )
   local _callback = callback
end

--[[---
Adds string to the list of completions. (To be called by the completion callback.)

TODO This is a stub implementation that doesn't work.

   @param completions The object passed to the completion callback.
   @tparam string option A possible completion.
--]]
function ln.addcompletion( completions, option )
   local _completions = completions
   local _option = option
end

return ln
