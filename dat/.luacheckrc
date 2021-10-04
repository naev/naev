-- vim: set expandtab shiftwidth=3 syntax=lua:

std = "lua51c+love+Basic"

-- The following standards correspond to nlua_load* functions.
-- Most of them just load one metatable, but some have dependencies, like nlua_loadGFX.
-- This is represented as: stds._GFX for the stuff loaded directly by nlua_loadGFX, and GFX representing all corresponding stds.

stds.Basic={
   globals={
      table={
         fields={"unpack"}
      },
      "warn", "_", "N_", "n_", "gettext",
   },
}
stds.AI = {read_globals={"ai"}} -- the C function is ai_loadProfile() in this case
stds.Audio = {read_globals={"audio"}}
stds.Background = {read_globals={"bkg"}}
stds.Camera = {read_globals={"camera"}}
stds.Canvas = {read_globals={"canvas"}}
stds.Col = {read_globals={"colour"}}
stds.Commodity = {read_globals={"commodity"}}
stds.Data = {read_globals={"data"}}
stds.Debug = {read_globals={"debug"}}
stds.Diff = {read_globals={"diff"}}
stds.Evt = {read_globals={"evt"}}
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
stds.Planet = {read_globals={"planet"}}
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
STANDARD = "+Naev+Var+Planet+System+Jump+Time+Player" .. PILOT .. "+Rnd+Diff+Faction+Vector+Outfit+Commodity+News+Shiplog+File+Data+Debug+LinOpt+Safelanes"
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
}
stds.API_comm = {globals={"comm"}}      -- C function: comm_openPilot()
stds.API_equip = {globals={"autoequip"}}-- C function: equipment_autoequipShip()
stds.API_faction = {globals={
   "faction_hit",                       -- C function: faction_modPlayerLua()
   "faction_player_enemy",              -- C function: faction_isPlayerEnemy()
   "faction_player_friend",             -- C function: faction_isPlayerFriend()
   "faction_standing_broad",            -- C function: faction_getStandingBroad()
   "faction_standing_text",             -- C function: faction_getStandingText()
}}
stds.API_land = {globals={
   "land",                              -- C function: planet_updateLand() -- and the rest come from XML files!
   "dv_mil_command",
   "dv_mil_restricted",
   "emp_mil_omega",
   "emp_mil_restricted",
   "emp_mil_wrath",
   "land_hiclass",
   "land_lowclass",
   "pir_clanworld",
   "ptn_mil_restricted",
   "srm_mil_kataka",
   "srm_mil_restricted",
   "srs_mil_mutris",
   "srs_mil_restricted",
   "thr_mil_restricted",
   "zlk_mil_restricted",
   "zlk_ruadan",
}}
stds.API_rescue = {globals={"rescue"}}  -- C function: land_stranded
stds.API_spawn = {globals={
   "create",                            -- C function: system_scheduler()
   "decrease",                          -- C function: system_rmCurrentPresence()
   "spawn",                             -- C function: system_scheduler()
}}
stds.Background.globals={"background", "renderbg", "renderfg", "renderov"}
stds.Evt.globals={"create"}
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
   "update_target",
}
stds.Misn.globals={"accept", "create"}
stds.Music.globals={"choose"}
stds.PilotOutfit.globals={
   "cleanup",
   "cooldown",
   "init",
   "onadd",
   "onhit",
   "onload",
   "onremove",
   "onscan",
   "onscanned",
   "onshoot",
   "onstealth",
   "ontoggle",
   "outofenergy",
   "update",
   "mem",
}

-- Finally, we use nlua_setenv() to make global variables part of the AI Profile and Pilot Outfit APIs.
table.insert(stds.PilotOutfit.globals, "mem")

files["ai/**/*.lua"].std = STANDARD .. "+AI"
files["autoequip.lua"].std = STANDARD .. TK .. "+API_equip"
files["bkg/*.lua"].std = STANDARD .. "+Tex+Col+Background+Camera"
files["comm.lua"].std = STANDARD .. "+API_comm"
files["events/**/*.lua"].std = STANDARD .. "+Evt+Hook+Camera+Tex+Background+Music+Audio" .. TK
files["factions/equip/*.lua"].std = STANDARD
files["factions/spawn/*.lua"].std = STANDARD .. "+API_spawn"
files["factions/standing/*.lua"].std = STANDARD .. "+API_faction"
files["gui/*.lua"].std = STANDARD .. GFX .. "+GUI" .. TK
files["landing.lua"].std = STANDARD .. "+API_land"
files["missions/**/*.lua"].std = STANDARD .. "+Misn+Hook+Camera+Tex+Background+Music+Audio" .. TK
files["outfits/**/*.lua"].std = STANDARD .. GFX .. "+PilotOutfit"
files["rescue.lua"].std = STANDARD .. TK .. "+API_rescue"
files["snd/music.lua"].std = STANDARD .. "+Music"
