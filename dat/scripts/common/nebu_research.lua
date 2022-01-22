--[[

   Helper functions for the nebula research campaign.

--]]
local vn = require 'vn'
local colour = require 'colour'
local mt = require 'merge_tables'
local fmt = require "format"

local nebu_research = {
    -- Main Characters
    student = {
        name = _("Robert Hofer"),
        portrait = "zalek/unique/student.webp",
        image = "gfx/portraits/zalek/unique/student.webp",
        colour = {0.9, 0.4, 0.1}, -- Orangish
    },
    mensing = {
        name = _("Dr. Mensing"),
        portrait = "zalek/unique/mensing.webp",
        image = "gfx/portraits/zalek/unique/mensing.webp",
        colour = {0.9, 0.2, 0.2}, -- Red
    },
    dvaered_officer = {
        name = _("Dvaered Officer"),
        portrait = "dvaered/dv_military_m8.webp",
        image = "gfx/portraits/dvaered/dv_military_m8.webp",
        colour = {0.7, 0.3, 0.05}, -- Orangish
    },
    empire_captain = {
        name = _("Empire Captain"),
        portrait = "empire/empire_mil_f5.webp",
        image = "gfx/portraits/empire/empire_mil_f5.webp",
        colour = {0.2, 0.7, 0.1}, -- greenish
    },
    log = function( text )
        shiplog.create( "zlk_neburesearch", _("Nebula Research"), _("Za'lek") )
        shiplog.append( "zlk_neburesearch", text )
    end
}

nebu_research.rewards = {
   credits00 = 300e3,
   credits01 = 800e3,
   credits02 = 400e3,
--   credits03 = 0e3, -- no credit reward, but may gives reputation
   credits04 = 500e3,
   credits05 = 800e3,
}



-- Helpers to create main characters
function nebu_research.vn_student( params )
    return vn.Character.new( nebu_research.student.name,
        mt.merge_tables( {
            image=nebu_research.student.image,
            color=nebu_research.student.colour,
        }, params) )
end
function nebu_research.vn_mensing( params )
    return vn.Character.new( nebu_research.mensing.name,
        mt.merge_tables( {
            image=nebu_research.mensing.image,
            color=nebu_research.mensing.colour,
        }, params) )
end
function nebu_research.vn_dvaered_officer( params )
    return vn.Character.new( nebu_research.dvaered_officer.name,
        mt.merge_tables( {
            image=nebu_research.dvaered_officer.image,
            color=nebu_research.dvaered_officer.colour,
        }, params) )
end
function nebu_research.vn_empire_captain( params )
    return vn.Character.new( nebu_research.empire_captain.name,
        mt.merge_tables( {
            image=nebu_research.empire_captain.image,
            color=nebu_research.empire_captain.colour,
        }, params) )
end

return nebu_research
