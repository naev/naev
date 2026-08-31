* Problem:    equipopt
* Class:      MIP
* Rows:       28
* Columns:    75 (75 integer, 75 binary)
* Non-zeros:  321
* Format:     Free MPS
*
NAME equipopt
ROWS
 N R0000000
 L CPU
 G energy_regen
 L mass
 L neg_energy_regen_mod
 L jammer
 L scrambler
 L hullcoat
 L emergency_shield_booster
 L Battery_I
 L Plasteel_Plating
 L Engine_Reroute
 L Reactor_Class_I
 L Improved_Stabilizer
 G Bolt_Weapon
 L Point_Defence
 L Manoeuvrability_Modifier
 L Launcher
 L s1-sum
 L s2-sum
 L s3-sum
 L s4-sum
 L s5-sum
 L s6-sum
 L s7-sum
 L s8-sum
 L s9-sum
 L s10-sum
 L s11-sum
COLUMNS
 M0000001 'MARKER' 'INTORG'
 s1-Battery_I R0000000 12.277688021 s1-sum 1
 s1-Battery_I mass 20 energy_regen -8.88178E-16
 s1-Plasteel_Plating R0000000 12.063975503 s1-sum 1
 s1-Plasteel_Plating mass 30 energy_regen -8.88178E-16
 s1-Engine_Reroute R0000000 16.742000804 s1-sum 1
 s1-Engine_Reroute Manoeuvrability_Modifier 1 neg_energy_regen_mod 1
 s1-Engine_Reroute energy_regen -3.57
 s1-Reactor_Class_I R0000000 11.325315977 s1-sum 1
 s1-Reactor_Class_I mass 25 energy_regen 4.25
 s1-Improved_Stabilizer R0000000 13.39402247 s1-sum 1
 s1-Improved_Stabilizer Manoeuvrability_Modifier 1 neg_energy_regen_mod 1
 s1-Improved_Stabilizer energy_regen -3.57
 s2-Battery_I R0000000 10.303333291 s2-sum 1
 s2-Battery_I mass 20 energy_regen -8.88178E-16
 s2-Plasteel_Plating R0000000 11.774947028 s2-sum 1
 s2-Plasteel_Plating mass 30 energy_regen -8.88178E-16
 s2-Engine_Reroute R0000000 14.580207096 s2-sum 1
 s2-Engine_Reroute Manoeuvrability_Modifier 1 neg_energy_regen_mod 1
 s2-Engine_Reroute energy_regen -3.57
 s2-Reactor_Class_I R0000000 9.7967954406 s2-sum 1
 s2-Reactor_Class_I mass 25 energy_regen 4.25
 s2-Improved_Stabilizer R0000000 10.075512958 s2-sum 1
 s2-Improved_Stabilizer Manoeuvrability_Modifier 1 neg_energy_regen_mod 1
 s2-Improved_Stabilizer energy_regen -3.57
 s3-Battery_I R0000000 10.570160277 s3-sum 1
 s3-Battery_I mass 20 energy_regen -8.88178E-16
 s3-Plasteel_Plating R0000000 13.107680634 s3-sum 1
 s3-Plasteel_Plating mass 30 energy_regen -8.88178E-16
 s3-Engine_Reroute R0000000 18.874526959 s3-sum 1
 s3-Engine_Reroute Manoeuvrability_Modifier 1 neg_energy_regen_mod 1
 s3-Engine_Reroute energy_regen -3.57
 s3-Reactor_Class_I R0000000 12.754306572 s3-sum 1
 s3-Reactor_Class_I mass 25 energy_regen 4.25
 s3-Improved_Stabilizer R0000000 11.001057218 s3-sum 1
 s3-Improved_Stabilizer Manoeuvrability_Modifier 1 neg_energy_regen_mod 1
 s3-Improved_Stabilizer energy_regen -3.57
 s4-Battery_I R0000000 12.013580499 s4-sum 1
 s4-Battery_I mass 20 energy_regen -8.88178E-16
 s4-Plasteel_Plating R0000000 10.64736448 s4-sum 1
 s4-Plasteel_Plating mass 30 energy_regen -8.88178E-16
 s4-Engine_Reroute R0000000 17.32734843 s4-sum 1
 s4-Engine_Reroute Manoeuvrability_Modifier 1 neg_energy_regen_mod 1
 s4-Engine_Reroute energy_regen -3.57
 s4-Reactor_Class_I R0000000 10.621763349 s4-sum 1
 s4-Reactor_Class_I mass 25 energy_regen 4.25
 s4-Improved_Stabilizer R0000000 9.1396607202 s4-sum 1
 s4-Improved_Stabilizer Manoeuvrability_Modifier 1 neg_energy_regen_mod 1
 s4-Improved_Stabilizer energy_regen -3.57
 s5-Emergency_Shield_Booster R0000000 10.57506332 s5-sum 1
 s5-Emergency_Shield_Booster emergency_shield_booster 1 mass 2
 s5-Emergency_Shield_Booster energy_regen -8.88178E-16
 s5-Unicorp_Scrambler R0000000 9.8728183762 s5-sum 1
 s5-Unicorp_Scrambler scrambler 1 mass 1
 s5-Unicorp_Scrambler energy_regen -8.88178E-16
 s5-Milspec_Impacto-Plastic_Coating R0000000 1237.0065852 s5-sum 1
 s5-Milspec_Impacto-Plastic_Coating hullcoat 1 energy_regen -8.88178E-16
 s5-Unicorp_Jammer R0000000 6.4388998201 s5-sum 1
 s5-Unicorp_Jammer jammer 1 mass 2
 s5-Unicorp_Jammer energy_regen -25
 s5-Sensor_Array R0000000 10.893627332 s5-sum 1
 s5-Sensor_Array energy_regen -8.88178E-16
 s6-Emergency_Shield_Booster R0000000 8.3498933629 s6-sum 1
 s6-Emergency_Shield_Booster emergency_shield_booster 1 mass 2
 s6-Emergency_Shield_Booster energy_regen -8.88178E-16
 s6-Unicorp_Scrambler R0000000 8.1551167296 s6-sum 1
 s6-Unicorp_Scrambler scrambler 1 mass 1
 s6-Unicorp_Scrambler energy_regen -8.88178E-16
 s6-Milspec_Impacto-Plastic_Coating R0000000 1053.0482819 s6-sum 1
 s6-Milspec_Impacto-Plastic_Coating hullcoat 1 energy_regen -8.88178E-16
 s6-Unicorp_Jammer R0000000 4.8366996349 s6-sum 1
 s6-Unicorp_Jammer jammer 1 mass 2
 s6-Unicorp_Jammer energy_regen -25
 s6-Sensor_Array R0000000 8.5969169803 s6-sum 1
 s6-Sensor_Array energy_regen -8.88178E-16
 s7-Emergency_Shield_Booster R0000000 9.3832647105 s7-sum 1
 s7-Emergency_Shield_Booster emergency_shield_booster 1 mass 2
 s7-Emergency_Shield_Booster energy_regen -8.88178E-16
 s7-Unicorp_Scrambler R0000000 8.7649891518 s7-sum 1
 s7-Unicorp_Scrambler scrambler 1 mass 1
 s7-Unicorp_Scrambler energy_regen -8.88178E-16
 s7-Milspec_Impacto-Plastic_Coating R0000000 1472.103983 s7-sum 1
 s7-Milspec_Impacto-Plastic_Coating hullcoat 1 energy_regen -8.88178E-16
 s7-Unicorp_Jammer R0000000 4.942935699 s7-sum 1
 s7-Unicorp_Jammer jammer 1 mass 2
 s7-Unicorp_Jammer energy_regen -25
 s7-Sensor_Array R0000000 8.5271429262 s7-sum 1
 s7-Sensor_Array energy_regen -8.88178E-16
 s8-Enygma_Systems_Turreted_Headhunter_Launcher R0000000 18.863805106 s8-sum 1
 s8-Enygma_Systems_Turreted_Headhunter_Launcher Launcher 1 mass 36
 s8-Enygma_Systems_Turreted_Headhunter_Launcher energy_regen -8.88178E-16 CPU 14
 s8-Enygma_Systems_Turreted_Fury_Launcher R0000000 19.238246691 s8-sum 1
 s8-Enygma_Systems_Turreted_Fury_Launcher Launcher 1 mass 30
 s8-Enygma_Systems_Turreted_Fury_Launcher energy_regen -8.88178E-16 CPU 12
 s8-Turreted_Gauss_Gun R0000000 8.0122853432 s8-sum 1
 s8-Turreted_Gauss_Gun Bolt_Weapon 1 mass 12
 s8-Turreted_Gauss_Gun energy_regen -1.034 CPU 24
 s8-Shredder R0000000 9.440788259 s8-sum 1
 s8-Shredder Bolt_Weapon 1 mass 14
 s8-Shredder energy_regen -3.055 CPU 72
 s8-Gauss_Gun R0000000 8.4519864733 s8-sum 1
 s8-Gauss_Gun Bolt_Weapon 1 mass 3
 s8-Gauss_Gun energy_regen -0.47 CPU 5
 s8-Mass_Driver R0000000 28.224159231 s8-sum 1
 s8-Mass_Driver Bolt_Weapon 1 mass 16
 s8-Mass_Driver energy_regen -4.935 CPU 50
 s8-TeraCom_Fury_Launcher R0000000 20.35434234 s8-sum 1
 s8-TeraCom_Fury_Launcher Launcher 1 mass 19
 s8-TeraCom_Fury_Launcher energy_regen -8.88178E-16 CPU 6
 s8-Ratchet_Point_Defence R0000000 15.185569268 s8-sum 1
 s8-Ratchet_Point_Defence Launcher 1 Point_Defence 1
 s8-Ratchet_Point_Defence mass 50 energy_regen -8.88178E-16
 s8-Ratchet_Point_Defence CPU 30
 s8-Turreted_Vulcan_Gun R0000000 17.320638831 s8-sum 1
 s8-Turreted_Vulcan_Gun Bolt_Weapon 1 mass 27
 s8-Turreted_Vulcan_Gun energy_regen -3.384 CPU 50
 s8-TeraCom_Headhunter_Launcher R0000000 18.428588328 s8-sum 1
 s8-TeraCom_Headhunter_Launcher Launcher 1 mass 23
 s8-TeraCom_Headhunter_Launcher energy_regen -8.88178E-16 CPU 7
 s8-Repeating_Banshee_Launcher R0000000 20.394268251 s8-sum 1
 s8-Repeating_Banshee_Launcher Launcher 1 mass 30
 s8-Repeating_Banshee_Launcher energy_regen -8.88178E-16 CPU 10
 s8-TeraCom_Mace_Launcher R0000000 8.2717123577 s8-sum 1
 s8-TeraCom_Mace_Launcher Launcher 1 mass 10
 s8-TeraCom_Mace_Launcher energy_regen -8.88178E-16 CPU 5
 s8-Vulcan_Gun R0000000 7.4183636811 s8-sum 1
 s8-Vulcan_Gun Bolt_Weapon 1 mass 7
 s8-Vulcan_Gun energy_regen -1.269 CPU 24
 s8-TeraCom_Banshee_Launcher R0000000 6.4593624685 s8-sum 1
 s8-TeraCom_Banshee_Launcher Launcher 1 mass 22
 s8-TeraCom_Banshee_Launcher energy_regen -8.88178E-16 CPU 5
 s9-Enygma_Systems_Turreted_Headhunter_Launcher R0000000 21.658882227 s9-sum 1
 s9-Enygma_Systems_Turreted_Headhunter_Launcher Launcher 1 mass 36
 s9-Enygma_Systems_Turreted_Headhunter_Launcher energy_regen -8.88178E-16 CPU 14
 s9-Enygma_Systems_Turreted_Fury_Launcher R0000000 17.8361329 s9-sum 1
 s9-Enygma_Systems_Turreted_Fury_Launcher Launcher 1 mass 30
 s9-Enygma_Systems_Turreted_Fury_Launcher energy_regen -8.88178E-16 CPU 12
 s9-Turreted_Gauss_Gun R0000000 7.8830093912 s9-sum 1
 s9-Turreted_Gauss_Gun Bolt_Weapon 1 mass 12
 s9-Turreted_Gauss_Gun energy_regen -1.034 CPU 24
 s9-Shredder R0000000 9.8434807472 s9-sum 1
 s9-Shredder Bolt_Weapon 1 mass 14
 s9-Shredder energy_regen -3.055 CPU 72
 s9-Gauss_Gun R0000000 8.6782080144 s9-sum 1
 s9-Gauss_Gun Bolt_Weapon 1 mass 3
 s9-Gauss_Gun energy_regen -0.47 CPU 5
 s9-Mass_Driver R0000000 28.886600942 s9-sum 1
 s9-Mass_Driver Bolt_Weapon 1 mass 16
 s9-Mass_Driver energy_regen -4.935 CPU 50
 s9-TeraCom_Fury_Launcher R0000000 21.58367514 s9-sum 1
 s9-TeraCom_Fury_Launcher Launcher 1 mass 19
 s9-TeraCom_Fury_Launcher energy_regen -8.88178E-16 CPU 6
 s9-Ratchet_Point_Defence R0000000 14.243565861 s9-sum 1
 s9-Ratchet_Point_Defence Launcher 1 Point_Defence 1
 s9-Ratchet_Point_Defence mass 50 energy_regen -8.88178E-16
 s9-Ratchet_Point_Defence CPU 30
 s9-Turreted_Vulcan_Gun R0000000 16.181765861 s9-sum 1
 s9-Turreted_Vulcan_Gun Bolt_Weapon 1 mass 27
 s9-Turreted_Vulcan_Gun energy_regen -3.384 CPU 50
 s9-TeraCom_Headhunter_Launcher R0000000 20.063630658 s9-sum 1
 s9-TeraCom_Headhunter_Launcher Launcher 1 mass 23
 s9-TeraCom_Headhunter_Launcher energy_regen -8.88178E-16 CPU 7
 s9-Repeating_Banshee_Launcher R0000000 25.398653019 s9-sum 1
 s9-Repeating_Banshee_Launcher Launcher 1 mass 30
 s9-Repeating_Banshee_Launcher energy_regen -8.88178E-16 CPU 10
 s9-TeraCom_Mace_Launcher R0000000 7.3683382677 s9-sum 1
 s9-TeraCom_Mace_Launcher Launcher 1 mass 10
 s9-TeraCom_Mace_Launcher energy_regen -8.88178E-16 CPU 5
 s9-Vulcan_Gun R0000000 10.008733758 s9-sum 1
 s9-Vulcan_Gun Bolt_Weapon 1 mass 7
 s9-Vulcan_Gun energy_regen -1.269 CPU 24
 s9-TeraCom_Banshee_Launcher R0000000 6.3094115437 s9-sum 1
 s9-TeraCom_Banshee_Launcher Launcher 1 mass 22
 s9-TeraCom_Banshee_Launcher energy_regen -8.88178E-16 CPU 5
 s10-Turreted_Gauss_Gun R0000000 14.793572537 s10-sum 1
 s10-Turreted_Gauss_Gun Bolt_Weapon 1 mass 12
 s10-Turreted_Gauss_Gun energy_regen -1.034 CPU 24
 s10-Shredder R0000000 19.136201465 s10-sum 1
 s10-Shredder Bolt_Weapon 1 mass 14
 s10-Shredder energy_regen -3.055 CPU 72
 s10-Gauss_Gun R0000000 13.802205931 s10-sum 1
 s10-Gauss_Gun Bolt_Weapon 1 mass 3
 s10-Gauss_Gun energy_regen -0.47 CPU 5
 s10-TeraCom_Mace_Launcher R0000000 14.756382037 s10-sum 1
 s10-TeraCom_Mace_Launcher Launcher 1 mass 10
 s10-TeraCom_Mace_Launcher energy_regen -8.88178E-16 CPU 5
 s10-Vulcan_Gun R0000000 19.30488273 s10-sum 1
 s10-Vulcan_Gun Bolt_Weapon 1 mass 7
 s10-Vulcan_Gun energy_regen -1.269 CPU 24
 s10-TeraCom_Banshee_Launcher R0000000 15.280078671 s10-sum 1
 s10-TeraCom_Banshee_Launcher Launcher 1 mass 22
 s10-TeraCom_Banshee_Launcher energy_regen -8.88178E-16 CPU 5
 s11-Turreted_Gauss_Gun R0000000 14.895074336 s11-sum 1
 s11-Turreted_Gauss_Gun Bolt_Weapon 1 mass 12
 s11-Turreted_Gauss_Gun energy_regen -1.034 CPU 24
 s11-Shredder R0000000 24.188993571 s11-sum 1
 s11-Shredder Bolt_Weapon 1 mass 14
 s11-Shredder energy_regen -3.055 CPU 72
 s11-Gauss_Gun R0000000 14.853188854 s11-sum 1
 s11-Gauss_Gun Bolt_Weapon 1 mass 3
 s11-Gauss_Gun energy_regen -0.47 CPU 5
 s11-TeraCom_Mace_Launcher R0000000 17.843504231 s11-sum 1
 s11-TeraCom_Mace_Launcher Launcher 1 mass 10
 s11-TeraCom_Mace_Launcher energy_regen -8.88178E-16 CPU 5
 s11-Vulcan_Gun R0000000 20.629228765 s11-sum 1
 s11-Vulcan_Gun Bolt_Weapon 1 mass 7
 s11-Vulcan_Gun energy_regen -1.269 CPU 24
 s11-TeraCom_Banshee_Launcher R0000000 12.623388189 s11-sum 1
 s11-TeraCom_Banshee_Launcher Launcher 1 mass 22
 s11-TeraCom_Banshee_Launcher energy_regen -8.88178E-16 CPU 5
 M0000002 'MARKER' 'INTEND'
RHS
 RHS1 CPU 51 energy_regen -10.65
 RHS1 mass 220.20477296 neg_energy_regen_mod 2
 RHS1 jammer 1 scrambler 1
 RHS1 hullcoat 1 emergency_shield_booster 1
 RHS1 Battery_I 3 Plasteel_Plating 3
 RHS1 Engine_Reroute 3 Reactor_Class_I 3
 RHS1 Improved_Stabilizer 3 Bolt_Weapon 1
 RHS1 Point_Defence 2 Manoeuvrability_Modifier 1
 RHS1 Launcher 3 s1-sum 1
 RHS1 s2-sum 1 s3-sum 1
 RHS1 s4-sum 1 s5-sum 1
 RHS1 s6-sum 1 s7-sum 1
 RHS1 s8-sum 1 s9-sum 1
 RHS1 s10-sum 1 s11-sum 1
BOUNDS
 UP BND1 s1-Battery_I 1
 UP BND1 s1-Plasteel_Plating 1
 UP BND1 s1-Engine_Reroute 1
 UP BND1 s1-Reactor_Class_I 1
 UP BND1 s1-Improved_Stabilizer 1
 UP BND1 s2-Battery_I 1
 UP BND1 s2-Plasteel_Plating 1
 UP BND1 s2-Engine_Reroute 1
 UP BND1 s2-Reactor_Class_I 1
 UP BND1 s2-Improved_Stabilizer 1
 UP BND1 s3-Battery_I 1
 UP BND1 s3-Plasteel_Plating 1
 UP BND1 s3-Engine_Reroute 1
 UP BND1 s3-Reactor_Class_I 1
 UP BND1 s3-Improved_Stabilizer 1
 UP BND1 s4-Battery_I 1
 UP BND1 s4-Plasteel_Plating 1
 UP BND1 s4-Engine_Reroute 1
 UP BND1 s4-Reactor_Class_I 1
 UP BND1 s4-Improved_Stabilizer 1
 UP BND1 s5-Emergency_Shield_Booster 1
 UP BND1 s5-Unicorp_Scrambler 1
 UP BND1 s5-Milspec_Impacto-Plastic_Coating 1
 UP BND1 s5-Unicorp_Jammer 1
 UP BND1 s5-Sensor_Array 1
 UP BND1 s6-Emergency_Shield_Booster 1
 UP BND1 s6-Unicorp_Scrambler 1
 UP BND1 s6-Milspec_Impacto-Plastic_Coating 1
 UP BND1 s6-Unicorp_Jammer 1
 UP BND1 s6-Sensor_Array 1
 UP BND1 s7-Emergency_Shield_Booster 1
 UP BND1 s7-Unicorp_Scrambler 1
 UP BND1 s7-Milspec_Impacto-Plastic_Coating 1
 UP BND1 s7-Unicorp_Jammer 1
 UP BND1 s7-Sensor_Array 1
 UP BND1 s8-Enygma_Systems_Turreted_Headhunter_Launcher 1
 UP BND1 s8-Enygma_Systems_Turreted_Fury_Launcher 1
 UP BND1 s8-Turreted_Gauss_Gun 1
 UP BND1 s8-Shredder 1
 UP BND1 s8-Gauss_Gun 1
 UP BND1 s8-Mass_Driver 1
 UP BND1 s8-TeraCom_Fury_Launcher 1
 UP BND1 s8-Ratchet_Point_Defence 1
 UP BND1 s8-Turreted_Vulcan_Gun 1
 UP BND1 s8-TeraCom_Headhunter_Launcher 1
 UP BND1 s8-Repeating_Banshee_Launcher 1
 UP BND1 s8-TeraCom_Mace_Launcher 1
 UP BND1 s8-Vulcan_Gun 1
 UP BND1 s8-TeraCom_Banshee_Launcher 1
 UP BND1 s9-Enygma_Systems_Turreted_Headhunter_Launcher 1
 UP BND1 s9-Enygma_Systems_Turreted_Fury_Launcher 1
 UP BND1 s9-Turreted_Gauss_Gun 1
 UP BND1 s9-Shredder 1
 UP BND1 s9-Gauss_Gun 1
 UP BND1 s9-Mass_Driver 1
 UP BND1 s9-TeraCom_Fury_Launcher 1
 UP BND1 s9-Ratchet_Point_Defence 1
 UP BND1 s9-Turreted_Vulcan_Gun 1
 UP BND1 s9-TeraCom_Headhunter_Launcher 1
 UP BND1 s9-Repeating_Banshee_Launcher 1
 UP BND1 s9-TeraCom_Mace_Launcher 1
 UP BND1 s9-Vulcan_Gun 1
 UP BND1 s9-TeraCom_Banshee_Launcher 1
 UP BND1 s10-Turreted_Gauss_Gun 1
 UP BND1 s10-Shredder 1
 UP BND1 s10-Gauss_Gun 1
 UP BND1 s10-TeraCom_Mace_Launcher 1
 UP BND1 s10-Vulcan_Gun 1
 UP BND1 s10-TeraCom_Banshee_Launcher 1
 UP BND1 s11-Turreted_Gauss_Gun 1
 UP BND1 s11-Shredder 1
 UP BND1 s11-Gauss_Gun 1
 UP BND1 s11-TeraCom_Mace_Launcher 1
 UP BND1 s11-Vulcan_Gun 1
 UP BND1 s11-TeraCom_Banshee_Launcher 1
ENDATA
