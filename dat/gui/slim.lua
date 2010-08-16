--[[
	First alternate GUI skin (Mockup)
--]]

function create()

	--Get player
	pp = player.pilot()
	pfact = pp:faction()
	pname = player.name()
	--pshhipname = player.ship()
	pship = pp:ship()
	
	--sys = system.cur()
	
	--Get sizes
	screen_w, screen_h = gfx.dim()
	deffont_h = gfx.fontSize()
	smallfont_h = gfx.fontSize(true)
	gui.viewport( 0, 30, screen_w, screen_h - 30 )
	
	--Colors
	col_txt_bar = colour.new( 192/255, 198/255, 217/255 )
	col_txt_top = colour.new( 148/255, 158/255, 192/255 )
	col_txt_std = colour.new( 111/255, 125/255, 169/255 )
	col_txt_wrn = colour.new( 127/255,  31/255,  31/255 )
	col_txt_enm = colour.new( 222/255,  28/255,  28/255 )
	col_txt_all = colour.new(  19/255, 152/255,  41/255 )
	col_txt_una = colour.new(  44/255,  48/255,  56/255 )
	col_shi = colour.new( 40/255,  51/255,  88/255 )
	col_arm = colour.new( 72/255,  73/255,  60/255 )
	col_ene = colour.new( 41/255,  92/255,  47/255 )
	col_spe = colour.new( 77/255,  80/255,  21/255 )
	col_spe2 = colour.new(169/255,177/255,  46/255 )
	
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
	bottom_bar = tex.open( base .. "bottombar.png" )
	warnlight1 = tex.open( base .. "warnlight1.png" )
	warnlight2 = tex.open( base .. "warnlight2.png" )
	warnlight3 = tex.open( base .. "warnlight3.png" )
	target_light_off = tex.open( base .. "targeted_off.png" )
	target_light_on =  tex.open( base .. "targeted_on.png" )
	--cargo_light_off = tex.open( base .. "cargo_off.png" )
	--cargo_light_on =  tex.open( base .. "cargo_on.png" )
	question = tex.open( base .. "question.png" )
	gui.targetPlanetGFX( tex.open( base .. "radar_planet.png" ) )
	gui.targetPilotGFX(  tex.open( base .. "radar_ship.png" ) )
	
	--Messages
	gui.mesgInit( screen_w - 400, 20, 50 )
	--OSD
	gui.osdInit( 30, screen_h-90, 150, 300 )
	
	--Get positions
	--Player pane
	pl_pane_w, pl_pane_h = player_pane:dim()
	pl_pane_x = screen_w - pl_pane_w - 16
	pl_pane_y = screen_h - pl_pane_h - 16
	
	--Radar
	radar_w, radar_h = radar_gfx:dim()
	radar_x = pl_pane_x - radar_w + 24
	radar_y = pl_pane_y + 13
	gui.radarInit( false, 124, 124 )
	
	
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
	-- ? image
	ta_question_w, ta_question_h = question:dim()
	
	--Targeted icon
	ta_icon_x = ta_pane_x + 82
	ta_icon_y = ta_pane_y + 100
	
	--Target Faction icon
	ta_fact_x = ta_pane_x + 114
	ta_fact_y = ta_pane_y + 100
	
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
	
	--Targeted warning light
	ta_warning_x = ta_pane_x + 82
	ta_warning_y = ta_pane_y + 100
	
	timers = {}
	timers[1] = 0.5
   timers[2] = 0.5
	blinkcol = col_txt_enm
   gfxWarn = true
	
	update_target()
	update_ship()
	update_system()
end

function update_target()

	ptarget = pp:target()
	if ptarget ~= nil then
		ptarget_gfx = ptarget:ship():gfxTarget()
		ptarget_gfx_w, ptarget_gfx_h = ptarget_gfx:dim()
		ptargetfact = ptarget:faction()
		ptarget_target = ptarget:target()
		ta_stats = ptarget:stats()
		
		ptarget_gfx_aspect = ptarget_gfx_w / ptarget_gfx_h
		
		if ptarget_gfx_aspect >= 1 then
			if ptarget_gfx_w > 92 then
				ptarget_gfx_draw_w = 92
				ptarget_gfx_draw_h = 92 / ptarget_gfx_w * ptarget_gfx_h
			end
		else
			if ptarget_gfx_h > 92 then
				ptarget_gfx_draw_h = 92
				ptarget_gfx_draw_w = 92 / ptarget_gfx_h * ptarget_gfx_w
			end
		end
		ptarget_faction_gfx = ptargetfact:logoTiny()
	end
end

function update_nav()
	nav_pnt, nav_hyp = pp:nav()
end

function update_cargo()
	cargol = player.cargoList()
	cargo = {}
	for k,v in ipairs(cargol) do
		if v.q == 0 then
			cargo[k] = v.name
		else
			cargo[k] = string.format( "%d"  .. "t %s", v.q, v.name )
		end
		if v.m then
			cargo[k] = cargo[k] .. "*"
		end
	end
end

function update_ship()
	stats = pp:stats()
end

function update_system()
	sys = system.cur()
end

function render( dt )
	
	--Radar
	gfx.renderTex( radar_gfx, radar_x, radar_y )
	gui.radarRender( radar_x + 2, radar_y + 2 )
	
	--Player pane
	gfx.renderTex( player_pane, pl_pane_x, pl_pane_y )
	
	--Values
	armor, shield = pp:health()
	energy = pp:energy()
	speed = pp:vel():dist()
	lockons = pp:lockon()
	autonav = player.autonav()
	sec, amm, rdy = pp:secondary()
	credits = player.credits()
	
	local col, small, txt
	--Shield
	if shield == 0. then
		col = col_txt_enm
	elseif shield <= 0.2 then
		col = col_txt_wrn
	else
		col = col_txt_bar
	end
	gfx.renderTex( bg_shi, shield_x + 30, shield_y + 2 )
	gfx.renderRect( shield_x + 30, shield_y + 2, shield/100. * bar_w, bar_h, col_shi )
	gfx.renderTex( bg_bar, shield_x, shield_y )
	gfx.renderTex( icon_shi, shield_x + 7, shield_y + 4 )
	gfx.renderTex( sheen, shield_x + 31, shield_y +15 )
	txt = tostring( math.floor(shield)) .. "% (" .. tostring(math.floor(stats.shield * shield / 100)) .. ")"
	if gfx.printDim( false, txt ) > bar_w then
		small = true
	else
		small = false
	end
	gfx.print( small, txt, shield_x + 30, shield_y + 6, col, bar_w, true)
	
	--Armor
	if armor <= 0.2 then
		col = col_txt_enm
	else
		col = col_txt_bar
	end
	gfx.renderTex( bg_arm, armor_x + 30, armor_y + 2 )
	gfx.renderRect( armor_x + 30, armor_y + 2, armor/100. * bar_w, bar_h, col_arm )
	gfx.renderTex( bg_bar, armor_x, armor_y )
	gfx.renderTex( icon_arm, armor_x + 7, armor_y + 4 )
	gfx.renderTex( sheen, armor_x + 31, armor_y +15 )
	txt = tostring( math.floor(armor)) .. "% (" .. tostring(math.floor(stats.armour * armor / 100)) .. ")"
	if gfx.printDim( false, txt ) > bar_w then
		small = true
	else
		small = false
	end
	gfx.print( small, txt, armor_x + 30, armor_y + 6, col, bar_w, true)
	
	--Energy
	if energy == 0. then
		col = col_txt_enm
	elseif energy <= 0.2 then
		col = col_txt_wrn
	else
		col = col_txt_bar
	end
	gfx.renderTex( bg_ene, energy_x + 30, energy_y + 2 )
	gfx.renderRect( energy_x + 30, energy_y + 2, energy/100. * bar_w, bar_h, col_ene )
	gfx.renderTex( bg_bar, energy_x, energy_y )
	gfx.renderTex( icon_ene, energy_x + 7, energy_y + 4 )
	gfx.renderTex( sheen, energy_x + 31, energy_y +15 )
	txt = tostring( math.floor(energy)) .. "% (" .. tostring(math.floor(stats.energy  * energy / 100)) .. ")"
	if gfx.printDim( false, txt ) > bar_w then
		small = true
	else
		small = false
	end
	gfx.print( small, txt, energy_x + 30, energy_y + 6, col_txt_bar, bar_w, true)
	
	--Speed
	local dispspe, dispspe2
	if math.floor(speed) > stats.speed then
		dispspe2 = speed/stats.speed - 1
		dispspe = 1
		col = col_txt_wrn
		if dispspe2 > 1 then
			dispspe2 = 1
			timers[1] = timers[1] - dt
			if timers[1] <= 0. then
				timers[1] = 0.5
				if blinkcol == col_txt_una then
					blinkcol = col_txt_enm
				else
					blinkcol = col_txt_una
				end
			end
			col = blinkcol
		end
	else
		dispspe = speed / stats.speed
		dispspe2 = 0
		col = col_txt_bar
	end
	gfx.renderTex( bg_spe, speed_x + 30, speed_y + 2 )
	if dispspe2 < 1 then
		gfx.renderRect( speed_x + 30, speed_y + 2, dispspe * bar_w, bar_h, col_spe )
	end
	if dispspe2 then
		gfx.renderRect( speed_x + 30, speed_y + 2, dispspe2 * bar_w, bar_h, col_spe2 )
	end
	gfx.renderTex( bg_bar, speed_x, speed_y )
	gfx.renderTex( icon_spe, speed_x + 7, speed_y + 4 )
	gfx.renderTex( sheen, speed_x + 31, speed_y +15 )
	txt = tostring( math.floor(speed / stats.speed * 100)) .. "% (" .. tostring( math.floor(speed)) .. ")"
	if gfx.printDim( false, txt ) > bar_w then
		small = true
	else
		small = false
	end
	gfx.print( small, txt, speed_x + 30, speed_y + 6, col, bar_w, true)

	--Warning Light
	if lockons > 0 then
      timers[2] = timers[2] - dt
      if timers[2] <= 0. then
         if lockons < 20 then
            timers[2] = 0.5 - (0.025 * lockons)
            gfxWarn = not gfxWarn
         else
            timers[2] = 0
            gfxWarn = true
         end
      end
      if gfxWarn == true then
         gfx.renderTex( warnlight1, pl_pane_x + 6, pl_pane_y + 115 )
      end
		local length
		length = gfx.printDim( false, "Warning - Missile Lockon detected" )
		gfx.print( false, "Warning - Missile Lockon detected", (screen_w - length)/2, screen_h - 100, col_txt_enm )
	end
	if armor <= 20 then
		gfx.renderTex( warnlight2, pl_pane_x + 29, pl_pane_y - 2 )
	end
	if autonav then
		gfx.renderTex( warnlight3, pl_pane_x + 162, pl_pane_y + 3 )
	end
	
	
	--Target Pane
	if ptarget ~= nil then
	
		ta_detect, ta_fuzzy = pp:inrange( ptarget )
		if ta_detect then
			
			--Frame
			gfx.renderTex( target_pane, ta_pane_x, ta_pane_y )
			gfx.renderTex( target_bg, ta_image_x, ta_image_y )
			
			local dispspe, dispspe2, specol
			if not ta_fuzzy then
            ptarget_target = ptarget:target()
				ta_armor, ta_shield, ta_disabled = ptarget:health()
				ta_energy = ptarget:energy()
				ta_speed = ptarget:vel():dist()
				ta_pos = ptarget:pos()
				ta_dist = pp:pos():dist( ta_pos )
				ta_dir = ptarget:dir()

				
				if math.floor(ta_speed) > ta_stats.speed then
					dispspe2 = ta_speed/ta_stats.speed - 1
					dispspe = 1
					specol = col_txt_wrn
					if dispspe2 > 1 then
						dispspe2 = 1
						specol = blinkcol
					end
				else
					dispspe = ta_speed / ta_stats.speed
					dispspe2 = 0
					specol = col_txt_bar
				end
				--Render target graphic
				if ptarget_gfx_w > 92 or ptarget_gfx_h > 92 then
					gfx.renderTexRaw( ptarget_gfx, ta_center_x - ptarget_gfx_draw_w / 2, ta_center_y - ptarget_gfx_draw_h / 2, ptarget_gfx_draw_w, ptarget_gfx_draw_h, 1, 1, 0, 0, 1, 1)
				else
					gfx.renderTex( ptarget_gfx, ta_center_x - ptarget_gfx_w / 2, ta_center_y - ptarget_gfx_h / 2)
				end
			else
				dispspe = 0
				dispspe2 = 0
				specol = col_txt_una
				--Render ?
				gfx.renderTex( question, ta_center_x - ta_question_w / 2, ta_center_y - ta_question_h / 2 )
			end
			
			--Title
			gfx.print( false, "TARGETED", ta_pane_x + 14, ta_pane_y + 180, col_txt_top )
			--Bars
			
			--Backgrounds
			gfx.renderTex( bg_shi_sm, shield_sm_x + 22, shield_sm_y + 2 )
			gfx.renderTex( bg_arm_sm, armor_sm_x + 22, armor_sm_y + 2 )
			gfx.renderTex( bg_ene_sm, energy_sm_x + 22, energy_sm_y + 2 )
			gfx.renderTex( bg_spe_sm, speed_sm_x + 22, speed_sm_y + 2 )
			
			--Actual bars
			if not ta_fuzzy then
				gfx.renderRect( shield_sm_x + 22, shield_sm_y + 2, ta_shield/100 * bar_sm_w, bar_sm_h, col_shi )
				gfx.renderRect( armor_sm_x + 22, armor_sm_y + 2, ta_armor/100 * bar_sm_w, bar_sm_h, col_arm )
				gfx.renderRect( energy_sm_x + 22, energy_sm_y + 2, ta_energy/100 * bar_sm_w, bar_sm_h, col_ene )
				if dispspe2 < 1 then
					gfx.renderRect( speed_sm_x + 22, speed_sm_y + 2, dispspe * bar_sm_w, bar_sm_h, col_spe )
				elseif dispspe2 then
					gfx.renderRect( speed_sm_x + 22, speed_sm_y + 2, dispspe2 * bar_sm_w, bar_sm_h, col_spe2 )
				end
			else
				gfx.renderRect( shield_sm_x + 22, shield_sm_y + 2, bar_sm_w, bar_sm_h, col_shi )
				gfx.renderRect( armor_sm_x + 22, armor_sm_y + 2, bar_sm_w, bar_sm_h, col_arm )
				gfx.renderRect( energy_sm_x + 22, energy_sm_y + 2, bar_sm_w, bar_sm_h, col_ene )
				--gfx.renderRect( speed_sm_x + 22, speed_sm_y + 2, bar_sm_w, bar_sm_h, col_spe )
			end
			
			--Metal Frames
			gfx.renderTex( bg_bar_sm, shield_sm_x, shield_sm_y )
			gfx.renderTex( bg_bar_sm, armor_sm_x, armor_sm_y )
			gfx.renderTex( bg_bar_sm, energy_sm_x, energy_sm_y )
			gfx.renderTex( bg_bar_sm, speed_sm_x, speed_sm_y )
			
			--Icons
			gfx.renderTex( icon_shi_sm, shield_sm_x + 5, shield_sm_y + 2 )
			gfx.renderTex( icon_arm_sm, armor_sm_x + 5, armor_sm_y + 2 )
			gfx.renderTex( icon_ene_sm, energy_sm_x + 5, energy_sm_y + 2 )
			gfx.renderTex( icon_spe_sm, speed_sm_x + 5, speed_sm_y + 2 )
			
			--Sheen			
			gfx.renderTex( sheen_sm, shield_sm_x + 23, shield_sm_y + 9 )
			gfx.renderTex( sheen_sm, armor_sm_x + 23, armor_sm_y + 9 )
			gfx.renderTex( sheen_sm, energy_sm_x + 23, energy_sm_y + 9 )
			gfx.renderTex( sheen_sm, speed_sm_x + 23, speed_sm_y + 9 )
			
			--Text, warning light & other texts
			if not ta_fuzzy then
				--Bar Texts
				gfx.print( false, tostring( math.floor(ta_shield) ) .. "% (" .. tostring(math.floor(ta_stats.shield  * ta_shield / 100)) .. ")", shield_sm_x + 22, shield_sm_y + 3, col_txt_bar, bar_sm_w, true )
				gfx.print( false, tostring( math.floor(ta_armor) ) .. "% (" .. tostring(math.floor(ta_stats.armour  * ta_armor / 100)) .. ")", armor_sm_x + 22, armor_sm_y + 3, col_txt_bar, bar_sm_w, true )
				gfx.print( false, tostring( math.floor(ta_energy) ) .. "%", energy_sm_x + 22, energy_sm_y + 3, col_txt_bar, bar_sm_w, true )
				gfx.print( false, tostring( math.floor(ta_speed / ta_stats.speed * 100 )) .. "% (" .. tostring( math.floor(ta_speed) ) .. ")", speed_sm_x + 22, speed_sm_y + 3, specol, bar_sm_w, true )
				
				--Warning Light
				if ptarget_target == pp then
					gfx.renderTex( target_light_on, ta_warning_x - 3, ta_warning_y - 3 )
				else
					gfx.renderTex( target_light_off, ta_warning_x, ta_warning_y )
				end
				
				--Faction Logo
				if ptarget_faction_gfx ~= nil then
					gfx.renderTex( ptarget_faction_gfx, ta_fact_x, ta_fact_y )
				end
				
				--Pilot name
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
			else
				--Bar Texts
				gfx.print( true, "UNAVAILABLE", shield_sm_x + 22, shield_sm_y + 3, col_txt_una, bar_sm_w, true )
				gfx.print( true, "UNAVAILABLE", armor_sm_x + 22, armor_sm_y + 3, col_txt_una, bar_sm_w, true )
				gfx.print( true, "UNAVAILABLE", energy_sm_x + 22, energy_sm_y + 3, col_txt_una, bar_sm_w, true )
				gfx.print( true, "UNAVAILABLE", speed_sm_x + 22, speed_sm_y + 3, col_txt_bar, bar_sm_w, true )
				
				--Warning light
				gfx.renderTex( target_light_off, ta_warning_x, ta_warning_y )
				
				--Pilot name
				gfx.print( true, "Unknown", ta_pane_x + 14, ta_pane_y + 166, col_txt_una )
			end
			
			--Dist
			gfx.print( true, "DIST", ta_pane_x + 130, ta_pane_y + 150, col_txt_top )
			if ta_dist ~= nil then
			   gfx.print( false, largeNumber( ta_dist ), ta_pane_x + 120, ta_pane_y +132, col_txt_std, 38, true )
         end
				
			--Dir
			gfx.print(true, "DIR", ta_pane_x + 86, ta_pane_y + 150, col_txt_top )
         if ta_dir ~= nil then
            gfx.print( false, tostring( math.floor(ta_dir) ), ta_pane_x + 86, ta_pane_y + 132, col_txt_std, 30, true)
         end
			
		end
	end
	
	
	--Bottom bar
	local length = 8
	gfx.renderTexRaw( bottom_bar, 0, 0, screen_w, 30, 1, 1, 0, 0, 1, 1 )
	
	gfx.print( false, "Player: ", length, 5, col_txt_top )
	length = length + gfx.printDim( false, "Player: " )
	
	gfx.print( true, pname, length, 6, col_txt_std )
	length = length + gfx.printDim( true, pname ) + 10
	
	gfx.print( false, "System: ", length, 5, col_txt_top )
	length = length + gfx.printDim( false, "System: " )
	
	gfx.print( true, sys:name(), length, 6, col_txt_std )
	length = length + gfx.printDim( true, sys:name() ) + 10
	
	gfx.print( false, "Credits: ", length, 5, col_txt_top )
	length = length + gfx.printDim( false, "Credits: " )
	
	gfx.print( true, largeNumber( credits ), length, 6, col_txt_std )
	length = length + gfx.printDim( true, largeNumber( credits ) ) + 10
	
	gfx.print( false, "Nav: ", length, 5, col_txt_top )
	length = length + gfx.printDim( false, "Nav: " )
	
	if nav_hyp ~= nil then
		local navstring = nav_hyp:name() .. " (" .. tostring(nav_hyp:jumpDist()) .. ")" 
		gfx.print( true, navstring, length, 6, col_txt_std )
		length = length + gfx.printDim( true, navstring ) + 10
	else
		gfx.print( true, "none", length, 6, col_txt_una )
		length = length + gfx.printDim( true, "none" ) + 10
	end
	
	gfx.print( false, "Planet: ", length, 5, col_txt_top )
	length = length + gfx.printDim( false, "Planet: " )
	
	if nav_pnt ~= nil then
		gfx.print( true, nav_pnt:name(), length, 6, col_txt_std )
		length = length + gfx.printDim( true, nav_pnt:name() ) + 10
	else
		gfx.print( true, "none", length, 6, col_txt_una )
		length = length + gfx.printDim( true, "none" ) + 10
	end
	
	gfx.print( false, "Secondary: ", length, 5, col_txt_top )
	length = length + gfx.printDim( false, "Secondary: " )
	
	if sec ~= nil then
		local col, secstr
		if rdy then
			col = col_txt_std
		else
			col = col_txt_una
		end
		secstr = sec
		if amm ~= nil then
			secstr = secstr .. " (" .. tostring(amm) .. ")"
		end
		gfx.print( true, secstr, length, 6, col )
		length = length + gfx.printDim( true, secstr ) + 10
	else
		gfx.print( true, "none", length, 6, col_txt_una )
		length = length + gfx.printDim( true, "none" ) + 10
	end
	
	if #cargo == 0 and length + gfx.printDim( false, "Cargo: " ) + gfx.printDim( true, "none" ) <= screen_w - 8 then
		gfx.print( false, "Cargo: ", length, 5, col_txt_top )
		length = length + gfx.printDim( false, "Cargo: " )
		gfx.print( true, "none", length, 6, col_txt_una )
		
	elseif #cargo == 1 and length + gfx.printDim( false, "Cargo: " ) + gfx.printDim( true, cargo[1] ) <= screen_w - 8 then
		gfx.print( false, "Cargo: ", length, 5, col_txt_top )
		length = length + gfx.printDim( false, "Cargo: " )
		gfx.print( true, cargo[1], length, 6, col_txt_std )
		
	elseif #cargo > 1 and length + gfx.printDim( false, "Cargo: " ) + gfx.printDim( true, cargo[1] .. "[...]" ) <= screen_w - 8 then
		gfx.print( false, "Cargo: ", length, 5, col_txt_top )
		length = length + gfx.printDim( false, "Cargo: " )
		
		gfx.print( true, cargo[1], length, 6, col_txt_std )
		length = length + gfx.printDim( true, cargo[1] )
		for k,v in pairs(cargo) do
			if k ~= 1 then
				if length + gfx.printDim( true, v ) > screen_w - 8 then
					gfx.print( true, "[...]", length, 6, col_txt_std )
					break
				else
					gfx.print( true, ", " .. v, length, 6, col_txt_std )
					length = length + gfx.printDim( true, v .. ", " )
				end
			end
		end
	end
	
end

function largeNumber( number )
	local formatted
	if number < 10000 then
		formatted = tostring(math.floor(number))
	elseif number < 1000000 then
		formatted = tostring( round( number / 1000, 2) ) .. "K"
	elseif number < 1000000000 then
		formatted = tostring( round( number / 1000000, 2) ) .. "M"
	elseif number < 1000000000000 then
		formatted = tostring( round( number / 1000000000, 2) ) .. "B"
	elseif number < 1000000000000000 then
		formatted = tostring( round( number / 1000000000000, 2) ) .. "T"
	else
		formatted = "Too big!"
	end
	return formatted
end

			
function round(num, idp)
	return tonumber(string.format("%." .. (idp or 0) .. "f", num))
end

function destroy()
end
