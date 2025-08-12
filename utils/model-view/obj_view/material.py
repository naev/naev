# SPDX-License-Identifier: GPL-3.0-or-later

from .texture import solidTexture

class Material:
    Ns = None
    Ka = None
    Kd = None
    Ks = None
    Ke = None
    Ni = None
    bm = 0.0
    d  = None

    def __init__(self):
        self.map_Kd = solidTexture(1, 1, 1)
        self.map_Ks = solidTexture(1, 1, 1)
        self.map_Ke = solidTexture(1, 1, 1)
        self.map_Tr = solidTexture(1, 1, 1)
        self.map_Bump = solidTexture(0, 0, 0)
