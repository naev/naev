# SPDX-License-Identifier: GPL-3.0-or-later

from .texture import solidTexture

class Material:
    Ka = None
    Kd = None
    Ks = None
    Ns = None
    Ni = None
    bm = 0.0
    d = 1.0

    def __init__(self):
       self.map_Kd = solidTexture(1, 1, 1)
       self.map_Bump = solidTexture(0, 0, 0)
