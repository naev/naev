--[[
	First alternate GUI skin (Mockup)
--]]

function create()

	--Get player
	pp = player.pilot()
	pfact = pp:faction()
	
	--Get sizes
	screen_w, screen_h = gfx.dim()
	deffont_h = gfx.fontSize()
	smallfont_h = gfx.fontSize(true)
	
	--Colors
	col_txt_bar = colour.new( 192/255, 198/255, 217/255 )
	col_txt_top = colour.new(  90/255, 111/255, 160/255 )
	col_txt_std = colour.new(  72/255,  83/255, 120/255 )
	col_txt_enm = colour.new( 222/255,  28/255,  28/255 )
	col_txt_all = colour.new(  19/255, 152/255,  41/255 )
	col_txt_una = colour.new(  90/255,  96/255, 114/255 )
	col_shi = colour.new( 40/255,  51/255,  88/255 )
	col_arm = colour.new( 72/255,  73/255,  60/255 )
	col_ene = colour.new( 41/255,  92/255,  47/255 )
	col_spe = colour.new( 77/255,  80/255,  21/255 )
	
	--Load Images
	local base = "gfx/gui/slim/"
	player_pane = tex.open( base .. "frame_player.png" )
	target_pane = tex.open( base .. "frame_target.png" )
	radar_gfx = tex.open( base .. "radar.png" )
	target_bg = tex.open( base .. "target_image.png" )
	icon_shi = tex.open( base .. "shield.png" )
	icon_arm = tex.open( base .. "armor.png" )
	icon_ene = tex.open( base .. "energy.png" )
	icon_spe = tex.open( base .. "speed.png" )
	icon_shi_sm = tex.open( base .. "shield_sm.png" )
	icon_arm_sm = tex.open( base .. "armor_sm.png" )
	icon_ene_sm = tex.open( base .. "energy_sm.png" )
	icon_spe_sm = tex.open( base .. "speed_sm.png" )
	bg_bar = tex.open( base .. "bg_bar.png" )
	bg_bar_sm = tex.open( base .. "bg_bar_sm.png" )
	bg_shi = tex.open( base .. "bg_shi.png" )
	bg_arm = tex.open( base .. "bg_arm.png" )
	bg_ene = tex.open( base .. "bg_ene.png" )
	bg_spe = tex.open( base .. "bg_spe.png" )
	bg_shi_sm = tex.open( base .. "bg_shi_sm.png" )
	bg_arm_sm = tex.open( base .. "bg_arm_sm.png" )
	bg_ene_sm = tex.open( base .. "bg_ene_sm.png" )
	bg_spe_sm = tex.open( base .. "bg_spe_sm.png" )
	sheen = tex.open( base .. "sheen.png" )
	sheen_sm = tex.open( base .. "sheen_sm.png" )
	warnlight2 = tex.open( base .. "warnlight2.png" )
	target_light_off = tex.open( base .. "targeted_off.png" )
	target_light_on =  tex.open( base .. "targeted_on.png" )
	gui.targetPlanetGFX( tex.open( base .. "radar_planet.png" ) )
	gui.targetPilotGFX(  tex.open( base .. "radar_ship.png" ) )
	
	--Get positions
	--Player pane
	pl_pane_w, pl_pane_h = player_pane:dim()
	pl_pane_x = screen_w - pl_pane_w - 16
	pl_pane_y = screen_h - pl_pane_h - 16
	
	--Radar
	radar_w, radar_h = radar_gfx:dim()
	radar_x = pl_pane_x - radar_w + 24
	radar_y = pl_pane_y + 13
	
	
	bar_w, bar_h = bg_shi:dim()
	--Shield Bar
	shield_x = pl_pane_x + 46
	shield_y = pl_pane_y + 102
	
	--Armor Bar
	armor_x = shield_x
	armor_y = shield_y - 28
	
	--Energy Bar
	energy_x = shield_x
	energy_y = armor_y - 28
	
	--Speed Bar
	speed_x = shield_x
	speed_y = energy_y - 28
	
	
	
	--Target Pane
	ta_pane_w, ta_pane_h = target_pane:dim()
	ta_pane_x = screen_w - ta_pane_w - 16
	ta_pane_y = pl_pane_y - ta_pane_h - 8
	
	--Target image background
	ta_image_x = ta_pane_x + 14
	ta_image_y = ta_pane_y + 96
	--Target image center
	ta_image_w, ta_image_h = target_bg:dim()
	ta_center_x = ta_image_x + ta_image_w / 2
	ta_center_y = ta_image_y + ta_image_h / 2
	
	--Targeted icon
	ta_icon_x = ta_pane_x + 82
	ta_icon_y = ta_pane_y + 104
	
	bar_sm_w, bar_sm_h = bg_shi_sm:dim()
	--Small Shield Bar
	shield_sm_x = ta_pane_x + 13
	shield_sm_y = ta_pane_y + 71
	
	--Small Armor Bar
	armor_sm_x = shield_sm_x
	armor_sm_y = shield_sm_y - 20
	
	--Small Energy Bar
	energy_sm_x = shield_sm_x
	energy_sm_y = armor_sm_y - 20
	
	--Small Speed Bar
	speed_sm_x = shield_sm_x
	speed_sm_y = energy_sm_y - 20
	
	update_target()
end

function update_target()

	ptarget = pp:target()
	if ptarget ~= nil then
		ptarget_gfx = ptarget:ship():gfxTarget()
		ptarget_gfx_w, ptarget_gfx_h = ptarget_gfx:dim()
		ptargetfact = ptarget:faction()
	end
end

function update_nav()
end

function render()
	
	--Radar
	gfx.renderTex( radar_gfx, radar_x, radar_y )
	
	--Player pane
	gfx.renderTex( player_pane, pl_pane_x, pl_pane_y )
	
	--Bars
	armor, shield = pp:health()
	energy = pp:energy()
	speed = pp:vel():dist()
	--Shield
	gfx.renderTex( bg_shi, shield_x + 30, shield_y + 2 )
	gfx.renderRect( shield_x + 30, shield_y + 2, shield/100. * bar_w, bar_h, col_shi )
	gfx.renderTex( bg_bar, shield_x, shield_y )
	gfx.renderTex( icon_shi, shield_x + 7, shield_y + 4 )
	gfx.renderTex( sheen, shield_x + 31, shield_y +15 )
	gfx.print( false, tostring( math.floor(shield)) .. "%", shield_x + 30, shield_y + 6, col_txt_bar, bar_w, true)
	--Armor
	gfx.renderTex( bg_arm, armor_x + 30, armor_y + 2 )
	gfx.renderRect( armor_x + 30, armor_y + 2, armor/100. * bar_w, bar_h, col_arm )
	gfx.renderTex( bg_bar, armor_x, armor_y )
	gfx.renderTex( icon_arm, armor_x + 7, armor_y + 4 )
	gfx.renderTex( sheen, armor_x + 31, armor_y +15 )
	gfx.print( false, tostring( math.floor(armor)) .. "%", armor_x + 30, armor_y + 6, col_txt_bar, bar_w, true)
	--Energy
	gfx.renderTex( bg_ene, energy_x + 30, energy_y + 2 )
	gfx.renderRect( energy_x + 30, energy_y + 2, energy/100. * bar_w, bar_h, col_ene )
	gfx.renderTex( bg_bar, energy_x, energy_y )
	gfx.renderTex( icon_ene, energy_x + 7, energy_y + 4 )
	gfx.renderTex( sheen, energy_x + 31, energy_y +15 )
	gfx.print( false, tostring( math.floor(energy)) .. "%", energy_x + 30, energy_y + 6, col_txt_bar, bar_w, true)
	--Speed
	gfx.renderTex( bg_spe, speed_x + 30, speed_y + 2 )
	gfx.renderRect( speed_x + 30, speed_y + 2, bar_w, bar_h, col_spe )
	gfx.renderTex( bg_bar, speed_x, speed_y )
	gfx.renderTex( icon_spe, speed_x + 7, speed_y + 4 )
	gfx.renderTex( sheen, speed_x + 31, speed_y +15 )
	gfx.print( false, tostring( math.floor(speed) ), speed_x + 30, speed_y + 6, col_txt_bar, bar_w, true)
	
	--Warning Light
	if armor <= 20 then
	   gfx.renderTex( warnlight2, pl_pane_x + 29, pl_pane_y - 2 )
	end
	
	if ptarget ~= nil then
		--Target Pane
		gfx.renderTex( target_pane, ta_pane_x, ta_pane_y )
		gfx.renderTex( target_bg, ta_image_x, ta_image_y )
		gfx.renderTex( ptarget_gfx, ta_center_x - ptarget_gfx_w / 2, ta_image_y - ptarget_gfx_h / 3)
		
		ta_armor, ta_shield, ta_disabled = ptarget:health()
		ta_energy = ptarget:energy()
		ta_speed = ptarget:vel():dist()
		ta_pos = ptarget:pos()
		ta_dist = pp:pos():dist( ta_pos )
		
		--Shield
		gfx.renderTex( bg_shi_sm, shield_sm_x + 22, shield_sm_y + 2 )
		gfx.renderRect( shield_sm_x + 22, shield_sm_y + 2, ta_shield/100 * bar_sm_w, bar_sm_h, col_shi )
		gfx.renderTex( bg_bar_sm, shield_sm_x, shield_sm_y )
		gfx.renderTex( icon_shi_sm, shield_sm_x + 5, shield_sm_y + 2 )
		gfx.renderTex( sheen_sm, shield_sm_x + 23, shield_sm_y + 9 )
		gfx.print( false, tostring( math.floor(ta_shield) ) .. "%", shield_sm_x + 22, shield_sm_y + 3, col_txt_bar, bar_sm_w, true )
		
		--Armor
		gfx.renderTex( bg_arm_sm, armor_sm_x + 22, armor_sm_y + 2 )
		gfx.renderRect( armor_sm_x + 22, armor_sm_y + 2, ta_armor/100 * bar_sm_w, bar_sm_h, col_arm )
		gfx.renderTex( bg_bar_sm, armor_sm_x, armor_sm_y )
		gfx.renderTex( icon_arm_sm, armor_sm_x + 5, armor_sm_y + 2 )
		gfx.renderTex( sheen_sm, armor_sm_x + 23, armor_sm_y + 9 )
		gfx.print( false, tostring( math.floor(ta_armor) ) .. "%", armor_sm_x + 22, armor_sm_y + 3, col_txt_bar, bar_sm_w, true )
		
		--Energy
		gfx.renderTex( bg_ene_sm, energy_sm_x + 22, energy_sm_y + 2 )
		gfx.renderRect( energy_sm_x + 22, energy_sm_y + 2, ta_energy/100 * bar_sm_w, bar_sm_h, col_ene )
		gfx.renderTex( bg_bar_sm, energy_sm_x, energy_sm_y )
		gfx.renderTex( icon_ene_sm, energy_sm_x + 5, energy_sm_y + 2 )
		gfx.renderTex( sheen_sm, energy_sm_x + 23, energy_sm_y + 9 )
		gfx.print( false, tostring( math.floor(ta_energy) ) .. "%", energy_sm_x + 22, energy_sm_y + 3, col_txt_bar, bar_sm_w, true )
		
		--Speed
		gfx.renderTex( bg_spe_sm, speed_sm_x + 22, speed_sm_y + 2 )
		gfx.renderRect( speed_sm_x + 22, speed_sm_y + 2, bar_sm_w, bar_sm_h, col_spe )
		gfx.renderTex( bg_bar_sm, speed_sm_x, speed_sm_y )
		gfx.renderTex( icon_spe_sm, speed_sm_x + 5, speed_sm_y + 2 )
		gfx.renderTex( sheen_sm, speed_sm_x + 23, speed_sm_y + 9 )
		gfx.print( false, tostring( math.floor(ta_speed) ), speed_sm_x + 22, speed_sm_y + 3, col_txt_bar, bar_sm_w, true )
		
		--Texts
		gfx.print( false, "TARGETED", ta_pane_x + 14, ta_pane_y + 180, col_txt_top )
		local ta_col
		if ta_disabled then
			col = col_txt_una
		else
			if pfact:areEnemies( ptargetfact ) then
				col = col_txt_enm
			elseif pfact:areAllies( ptargetfact ) then
				col = col_txt_all
			else
				col = col_txt_std
			end
		end
		gfx.print( true, ptarget:name(), ta_pane_x + 14, ta_pane_y + 166, col )
		
		gfx.print( true, "DIST", ta_pane_x + 120, ta_pane_y + 150, col_txt_top )
		gfx.print( false, largeNumber( ta_dist ), ta_pane_x + 110, ta_pane_y +130, col_txt_std, 38, true )
	end
end

function largeNumber( number )
	local formatted
	if number < 10000 then
		formatted = tostring(math.floor(number))
	elseif number < 1000000 then
		formatted = tostring( round( number, 1) ) .. "K"
	elseif number < 1000000000 then
		formatted = tostring(round( number, 2) ) .. "M"
	elseif number < 1000000000000 then
		formatted = tostring(round( number, 2) ) .. "B"
	elseif number < 1000000000000000 then
		formatted = tostring(round( number, 2) ) .. "T"
	end
	return formatted
end

			
function round(num, idp)
	return tonumber(string.format("%." .. (idp or 0) .. "f", num))
end

function destroy()
end