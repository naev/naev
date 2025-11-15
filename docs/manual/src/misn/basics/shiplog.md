# Ship Log

The Ship Log is a framework that allows recording in-game events so that the player can easily access them later on.
This is meant to help players that haven't logged in for a while or have forgotten what they have done in their game.
The core API is in the [`shiplog` module](https://naev.org/api/modules/shiplog.html) and is a core library that is always loaded without the need to `require`.
It consists of two functions:

* `shiplog.create`: takes three parameters, the first specifies the ID of the log (string), the second the name of the log (string, visible to player), and the third is the logtype (string, visible to player and used to group logs).
* `shiplog.append`: takes two parameters, the first specifies the ID of the log (string), and second is the message to append.
  The ID should match one created by `shiplog.create`.

The logs have the following hierarchy: logtype → log name → message.
The logtype and log name are specified by `shiplog.create` and the messages are added with `shiplog.append`.
Since, by default, `shiplog.create` doesn't overwrite existing logs, it's very common to write a helper log function as follows:

```lua
local function addlog( msg )
   local logid = "my_log_id"
   shiplog.create( logid, _("Secret Stuff"), _("Neutral") )
   shiplog.append( logid, msg )
end
```

You would use the function to quickly add log messages with `addlog(_("This is a message relating to secret stuff."))`.
Usually logs are added when important one-time things happen during missions or when they are completed.
