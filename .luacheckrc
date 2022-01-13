-- vim: set expandtab shiftwidth=3 syntax=lua:

std = "lua51+love+Basic"

-- It would be nice to enforce a ~120 char limit for regular lines of code, but missions etc. have inline text of considerable size.
-- Note: there's a `max_string_line_length` option, but that only applies when the line ending is inside a string. Useless.
max_line_length = 4096

-- Ignore unused variables whose names start with "_".
-- The default convention allows "_" itself as an unused variable, but we reserve that for gettext.
-- Furthermore, ignore all unused loop variables.
-- Finally, don't complain an _-prefixed variable is "never accessed" (e.g., if you set the tuple _foo, bar twice).
-- Iteration idioms in Lua all but require ipairs(), and whether we happen to use the key or value isn't fundamental.
ignore = {"21./_.*", "213", "231/_.*"}

-- The following standards correspond to nlua_load* functions.
-- Most of them just load one metatable, but some have dependencies, like nlua_loadGFX.
-- This is represented as: stds._GFX for the stuff loaded directly by nlua_loadGFX, and GFX representing all corresponding stds.

stds.Basic={
   globals={
      table={
         fields={"unpack"}
      },
      "_LOADED", -- NLUA_LOAD_TABLE
      "inlist",
   },
   read_globals={"N_", "_", "__debugging", "gettext", "n_", "warn"},
}
stds.AI = {read_globals={"ai"}} -- the C function is ai_loadProfile() in this case
stds.Audio = {read_globals={"audio"}}
stds.Background = {read_globals={"bkg"}}
stds.Camera = {read_globals={"camera"}}
stds.Canvas = {read_globals={"canvas"}}
stds.Col = {read_globals={"colour"}}
stds.Commodity = {read_globals={"commodity"}}
stds.Data = {read_globals={"data"}}
stds.Diff = {read_globals={"diff"}}
stds.Evt = {read_globals={"evt"}}
stds.CLI = {read_globals={"script", "printRaw"}} -- Actually set in cli_initLua()
stds.Faction = {read_globals={"faction"}}
stds.File = {read_globals={"file"}}
stds.Font = {read_globals={"font"}}
stds._GFX = {read_globals={"gfx"}}
stds.GUI = {read_globals={"gui"}}
stds.Hook = {read_globals={"hook"}}
stds.Jump = {read_globals={"jump"}}
stds.LinOpt = {read_globals={"linopt"}}
stds.Misn = {read_globals={"misn"}}
stds.Music = {read_globals={"music"}}
stds.Naev = {read_globals={"naev"}}
stds.News = {read_globals={"news"}}
stds.Outfit = {read_globals={"outfit"}}
stds._Pilot = {read_globals={"pilot"}}
stds.PilotOutfit = {read_globals={"pilotoutfit"}}
stds.Spob = {read_globals={"spob"}}
stds.Player = {read_globals={"player"}}
stds.Rnd = {read_globals={"rnd"}}
stds.Safelanes = {read_globals={"safelanes"}}
stds.Shader = {read_globals={"shader"}}
stds.Ship = {read_globals={"ship"}}
stds.Shiplog = {read_globals={"shiplog"}}
stds.System = {read_globals={"system"}}
stds.Tex = {read_globals={"tex"}}
stds.Time = {read_globals={"time"}}
stds._Tk = {read_globals={"tk"}}
stds.Transform = {read_globals={"transform"}}
stds.Var = {read_globals={"var"}}
stds.Vector = {read_globals={"vec2"}}

PILOT = "+_Pilot+Ship"
STANDARD = "+Naev+Var+Spob+System+Jump+Time+Player" .. PILOT .. "+Rnd+Diff+Faction+Vector+Outfit+Commodity+News+Shiplog+File+Data+LinOpt+Safelanes"
GFX = "+_GFX+Col+Tex+Font+Transform+Shader+Canvas"
TK = "+_Tk+Col" .. GFX

-- In addition, the Naev code base looks for APIs *exported* by certain types of scripts:
-- This is done using nlua_refenv-family calls for exported functions, and nlua_getenv/nlua_setenv for magic variables.
stds.AI.globals = {
   "attacked",
   "attacked_manual",
   "control",
   "control_manual",
   "control_rate",
   "create",
   "discovered",
   "distress",
   "hail",
   "mem",
   "refuel",
   -- Also, numerous tasks are brought into the API by nlua_pilot.c functions:
   "attack_forced",             -- pilotL_attack
   "brake",                     -- pilotL_brake
   "face",                      -- pilotL_face
   "face_towards",              -- pilotL_face
   "follow",                    -- pilotL_follow
   "follow_accurate",           -- pilotL_follow
   "gather",                    -- pilotL_gather
   "hyperspace",                -- pilotL_hyperspace
   "hyperspace_shoot",          -- pilotL_hyperspace
   "land",                      -- pilotL_land
   "land_shoot",                -- pilotL_land
   "moveto",                    -- pilotL_moveto
   "moveto_nobrake",            -- pilotL_moveto
   "moveto_nobrake_raw",        -- pilotL_moveto
   "lunge",                     -- the_bite
   "runaway",                   -- pilotL_runaway
   "runaway_jump",              -- pilotL_runaway
   "runaway_land",              -- pilotL_runaway
   "runaway_nojump",            -- pilotL_runaway
   "stealth",                   -- pilotL_stealth
   -- FIXME: Internal APIs formed by dat/ai code that are very difficult to untangle...
   "control_funcs",             -- a shared dispatch table...
   "create_post",               -- the standardized final step of every create()
   "idle",                      -- a task, commonly called and overridden
   "should_attack",             -- discretion may or may not be the better part of valor. Everyone gets an opinion.
   "taunt",                     -- everyone has their own!
   "transportParam",            -- initialization helper for the AIs
}
stds.API_board = {globals={"board"}}    -- C function: player_board()
stds.API_comm = {globals={"comm"}}      -- C function: comm_openPilot()
stds.API_loadscreen = {globals={"render"}}    -- C function: loadscreen_render()
stds.API_autoequip = {globals={"autoequip"}}            -- C function: equipment_autoequipShip()
stds.API_equip = {globals={"equip", "equip_generic"}}   -- C function: ai_create
stds.API_faction = {globals={
   "standing",                          -- C functions: faction_{modPlayerLua,isPlayerFriend,getStandingBroad,getStandingText}()
}}
stds.API_land = {globals={"land"}}      -- C function: planet_updateLand()
stds.API_rescue = {globals={"rescue"}}  -- C function: land_stranded
stds.API_save_updater = {globals={
   "license",                           -- C function: player_tryAddLicense
   "outfit",                            -- C function: player_tryGetOutfit
}}
stds.API_shipai = {globals={"create"}}  -- C function: info_shipAI
stds.API_spawn = {globals={
   "create",                            -- C function: system_scheduler()
   "decrease",                          -- C function: system_rmCurrentPresence()
   "spawn",                             -- C function: system_scheduler()
}}
stds.API_spob = {globals={
   "load",     -- C function: planet_gfxLoad
   "unload",   -- C function: planet_gfxUnload
   "can_land", -- C function: planet_updateLand
   "land",     -- C function: player_land
   "render",   -- C function: space_renderSpob
   "update",   -- C function: space_updateSpob
}}
stds.Background.globals={"background", "renderbg", "rendermg", "renderfg", "renderov"}
stds.Evt.globals={"create", "mem"}
stds.GUI.globals = {
   "create",
   "end_cooldown",
   "mouse_click",
   "mouse_move",
   "render",
   "render_cooldown",
   "update_cargo",
   "update_faction",
   "update_nav",
   "update_ship",
   "update_system",
   "update_effects",
   "update_target",
}
stds.Misn.globals={"abort", "accept", "create", "mem"}
stds.Music.globals={"choose"}
stds.PilotOutfit.globals={
   "cleanup",
   "cooldown",
   "init",
   "land",
   "onadd",
   "onhit",
   "onimpact",
   "onload",
   "onremove",
   "onscan",
   "onscanned",
   "onshoot",
   "onstealth",
   "ontoggle",
   "outofenergy",
   "takeoff",
   "jumpin",
   "update",
   "mem", -- Automatically created using nlua_setenv().
}

files["dat/ai/**/*.lua"].std = STANDARD .. "+AI"
files["dat/autoequip.lua"].std = STANDARD .. TK .. "+API_autoequip"
files["dat/bkg/**/*.lua"].std = STANDARD .. "+Tex+Col+Background+Camera+Audio" .. GFX
files["dat/board.lua"].std = STANDARD .. "+API_board"
files["dat/comm.lua"].std = STANDARD .. "+API_comm"
files["dat/common.lua"].std = STANDARD
files["dat/loadscreen.lua"].std = STANDARD .."+API_loadscreen"
files["dat/events/**/*.lua"].std = STANDARD .. "+Evt+Hook+Camera+Tex+Background+Music+Audio" .. TK
files["dat/factions/equip/*.lua"].std = STANDARD .. "+API_equip"
files["dat/factions/spawn/**/*.lua"].std = STANDARD .. "+API_spawn"
files["dat/factions/standing/**/*.lua"].std = STANDARD .. "+API_faction"
files["dat/gui/*.lua"].std = STANDARD .. GFX .. "+GUI" .. TK
files["dat/landing.lua"].std = STANDARD .. "+API_land"
files["dat/lua-repl/**/*.lua"].only = {}  -- not our code, so shut up, please
files["dat/missions/**/*.lua"].std = STANDARD .. "+Misn+Hook+Camera+Tex+Background+Music+Audio" .. TK
files["dat/outfits/**/*.lua"].std = STANDARD .. GFX .. "+PilotOutfit+Camera"
files["dat/rescue.lua"].std = STANDARD .. TK .. "+API_rescue"
files["dat/rep.lua"].std = STANDARD .. TK .. "+Tex+Col+Background+CLI+Camera+Music+Audio+LinOpt"
files["dat/save_updater.lua"].std = "API_save_updater"
files["dat/shipai.lua"].std = STANDARD .. TK .. "+API_shipai"
files["dat/snd/music.lua"].std = STANDARD .. "+Music"
files["dat/spob/**/*.lua"].std = STANDARD .. GFX .."+Camera+API_spob"

-- No way to be sure what type of environment will load these.
files["dat/scripts/**/*.lua"].std = STANDARD .. TK .. "+Misn+Hook+Camera+Tex+Background+Music+Audio" .. TK
