#!/usr/bin/env python3
import helper as h
N_ = lambda text: text
data = h.read()

data['outfit']['@name'] = N_('Neutralizer')

general = data['outfit']['general']
general['rarity'] = 2
general['description'] = N_("Based on the Heavy Ion Cannon, the Neutralizer is proof to the importance of specialized engineering. With most of the hardware and software significantly modified or replaced, the Neutralizer shows significant improvements in nearly all facets. Due to the fact that few were made, it is a fairly rare find, but an import asset in a pilot's arsenal.")
general['$price'] += 150e3
general['$mass'] += -2
general['$cpu'] = round(general['$cpu'] * 0.92)

specific = data['outfit']['specific']
specific['$range'] *= 1.1
specific['$falloff'] *= 1.2
specific['$delay'] *= 1.05
specific['$energy'] *= 1.05
specific['$swivel'] *= 1.5
specific['$trackmax'] = round(specific['$trackmax'] * 0.8333333333)
specific['lua'] = 'outfits/lib/matrix_sell.lua'

damage = specific['damage']
damage['$disable'] *= 1.2
damage['$physical'] *= 1.25
damage['$penetrate'] = round(damage['$penetrate'] * 1.15)

data.save()
