#!/usr/bin/env python3

# Generates polygons from image files

# WARNING : this script uses python 3
# For needed depandancies, please see the import section

# If you added a space ammo/bolt png:
# Go to bottom of the file, and replace '../naev/dat/gfx/outfit/space/' and
# '../naev/dat/gfx/outfit/space_polygon/' by the right paths in naev repo
# Then, commertarize the line "polygonify_all_ships" and run this script
# from shell (or any other interface that prints warnings).
# If the script refines the polygon a lot (more than 3-4 times), there
# is maybe something wrong. You'll have to add your file to special files
# in "polygonify_all_outfits" function. To see what parameters to put,
# read the rest of this text, or request help on discord.

# If you added a ship png:
# Go to bottom of the file, and replace '../naev/dat/gfx/ship/' and
# '../naev/dat/gfx/ship_polygon/' by the right paths in naev repo
# Then, commentarize the line "polygonify_all_outfits".
# If your png has more than 8X8 sprites,
# go in the function "polygonify_all_ships" and add the name of your
# png in the dictionnary maxNmin, along with (sx,sy,150,3,6).
# If your sprite is very big (2048), replace 3,6 by 4,8
# Then,run the script from shell
# (or any other interface that prints warnings).
# If the script refines the polygon a lot (more than 10-15 times), there
# is maybe something wrong. To see what parameters to use,
# read the rest of this text, or request help on discord.

# Generating data for all outfits :
# polygonify_all_outfits( origin_address, destination_address, overwrite )

# Generating data for all ships :
# polygonify_all_ships( origin_address, destination_address, overwrite )

# If the third argument (overwrite) is 0, only new file will be generated
# if overwrite == 1, all the polygons will be generated

# Using the algorithm for one png :

# 1)Run polygonFromPng('address_of_my_png',sx,sy,150,3,6)
# sx and sy are the nb of sprites in the picture (ex for Lancelot, its 8 and 8)
# If the code doesn't give any warning, run generateXML and put the generated
# file in ship_polygon or space_polygon directory.
# The last two arguments are the min and max length of the polygon's faces.
# Usually, I use (3,6) for ships and (2,4) for outfits.

# 2)If the set of points is not connex, while the png is, decrease the ceil
# (4th argument of polygonFromPng, down to 1 if needed)

# 3)If the code says that sx or sy are wrong, check that the values you have
# given are right. If they are, as written in the warning message, the error
# will not be greater as 1 pixel.

# 4)For extremely big ships, like the diablo, it could be interesting to use
# coarser dimensions like 4 and 8 for the last two arguments

# 5)For purposely non-connex objects (like the ripper shot), the aim is to
# build a polygon that contains all the parts of the object. For that purpose,
# increase the last 2 arguments up to (4,8), or even (5,10).
# Remark: for these cases, a convex polygon algorithm could be preferable...

# /!\ Very important : once you've found the right parameters to generate the
# polygon from your png, add them in the dictionnary maxNmin
# (except if the default values are right)


# Principle of the algorithm :

# 1 ) The transparency array is transformed into a set of points in
# PointsFromPng.
# Each point is adjacent to 4 pixels. In order for that point
# to be activated, at least 1 of the 4 pixels must have an opacity value that
# is greater than the ceil (150 for most ships, 1 for Zlk ships that have
# very small features and could end up non-connex otherwise)

# Rem 1 : If all the pixels have opacity>=ceil, the point is not activated as
# this point is for sure not on the boundary and only boundary are important
# in what we want to do

# Rem 2 : The value of ceil of 150 is totally arbitrary

# 2 ) The set of points is transformed into a polygon in polygonFromPng.
# The algo picks up one of the rightmost points. This point is the starting
# point. We define as well the starting direction as nearly vertical
#   Then the following recurcive algo runs :
#   a/ From the current point, find the next point that is at a distance
#      between minlen and maxlen and that makes the minimal angle with the
#      previous direction.
#   b/ Compute the next direction in order to prevent going backwards.
#      This direction makes the maximal angle with the direction going from
#      current point to previous point, and is chosen among the points that
#      are at a distance < maxlen from the previous point.
#      This step is necessary in the concave parts of ships.
#   After that, a few checks are performed. If the generated polygon fails,
#   a new finer polygon is generated.

# 3 ) The polygon is simplified in simplifyPolygon.
# A loop is run among the points of the polygon. Any point which angle is too
# close to pi is suppressed.

import numpy as np
import math
import matplotlib.pyplot as plt
import xml.etree.ElementTree as ET
import xml.dom.minidom as pretty
import os

# Create an array from the picture
def arrFromPng( address,sx,sy ):

    buffer = plt.imread(address)

    if np.shape(buffer)[2] == 4:
        picture = buffer[:,:,3]
    elif np.shape(buffer)[2] == 2: # Black and white
        picture = buffer[:,:,1]
    else:
        print('Warning: unable to read png file')
        picture = buffer[:,:,3] # Try this, maybe it will work

    # Now see if values are in [0,1] or [0,255]
    if np.max(picture) <= 1:
        picture = 255*picture

    # Store all the different pictures in a list of matrices
    six = picture.shape[0]/sx
    siy = picture.shape[1]/sy

    if (int(six) != six) or (int(siy) != siy) :
        print(('Warning: sx = ' + str(sx) + ' or sy = ' + str(sy) + \
               ' is wrong. The shape may be up to 1 pixel wrong.'))

    six = int(six)
    siy = int(siy)

    # For the case when sx or sy is wrong, all the pictures have not the
    # same size
    stax = list(range(sx))
    endx = list(range(sx))
    for i in range(sx):
        stax[i] = int( i * picture.shape[0]/sx )
        endx[i] = int( (i+1) * picture.shape[0]/sx )

    stay = list(range(sy))
    endy = list(range(sy))
    for i in range(sy):
        stay[i] = int( i * picture.shape[1]/sy )
        endy[i] = int( (i+1) * picture.shape[1]/sy )

    # Create and populate the list of matrices
    pictensor = list(range(sx*sy))
    for i in range(sx):
        for j in range(sy):
            sxy = sy*i + j
            pictensor[sxy] = picture[ stax[i]:endx[i], stay[j]:endy[j] ]

    return pictensor


# Defines points from png
def pointsFromPng(address,sx,sy,ceil):

    picture = arrFromPng(address,sx,sy)
    npict = len(picture)

    pointsx = [] # This list of list will contian the abscissae of the points
    pointsy = []

    for p in range(npict):
        pictcur = picture[p]

        ssx = pictcur.shape[0]
        ssy = pictcur.shape[1]
        sx2 = ssx/2 # Offset value
        sy2 = ssy/2

        bufferx = [] # TODO : something to pre-allocate memory
        buffery = []

        # loop over the corners (between pixels) if at least one pixel adjacent
        # to corner is > ceil, put a point at this corner
        # + + + + + + + +
        #  0 1 1 1 0 0 0
        # + + + + + + + +
        #  0 0 1 1 1 0 0
        # + + + + + + + +

        for i in range(ssx+1): # There is 1 corner more than pixels
            for j in range(ssy+1):
                if i==0:
                    if j==0:
                        if pictcur[0,0] >= ceil:
                            bufferx.append(-i+sx2) # -i because image vs coordinates
                            buffery.append(j-sy2)
                    elif j==ssy:
                        if pictcur[0,ssy-1] >= ceil:
                            bufferx.append(-i+sx2)
                            buffery.append(j-sy2)
                    else:
                        if pictcur[0,j-1] >= ceil or pictcur[0,j] >= ceil:
                            bufferx.append(-i+sx2)
                            buffery.append(j-sy2)

                elif i==ssx:
                    if j==0:
                        if pictcur[ssx-1,0] >= ceil:
                            bufferx.append(-i+sx2)
                            buffery.append(j-sy2)
                    elif j==ssy:
                        if pictcur[ssx-1,ssy-1] >= ceil:
                            bufferx.append(-i+sx2)
                            buffery.append(j-sy2)
                    else:
                        if pictcur[ssx-1,j-1] >= ceil or pictcur[ssx-1,j] >= ceil:
                            bufferx.append(-i+sx2)
                            buffery.append(j-sy2)

                else:
                    if j==0:
                        if pictcur[i-1,0] >= ceil or pictcur[i,0] >= ceil:
                            bufferx.append(-i+sx2)
                            buffery.append(j-sy2)
                    elif j==ssy:
                        if pictcur[i-1,ssy-1] >= ceil or pictcur[i,ssy-1] >= ceil:
                            bufferx.append(-i+sx2)
                            buffery.append(j-sy2)
                    else:
                        # This is the most general case. Remove points inside the domain
                        if (pictcur[i-1,j-1] >= ceil or pictcur[i-1,j] >= ceil \
                            or pictcur[i,j-1] >= ceil or pictcur[i,j] >= ceil) \
                            and not (pictcur[i-1,j-1] >= ceil and pictcur[i-1,j] >= ceil \
                            and pictcur[i,j-1] >= ceil and pictcur[i,j] >= ceil):
                            bufferx.append(-i+sx2)
                            buffery.append(j-sy2)

        pointsx.append(buffery)
        pointsy.append(bufferx) # We invert because pictures and matrix dont use the same coordinate system

    return (pointsx,pointsy)

# Simplify a polygon by removing points that are aligned with other points
def simplifyPolygon(indices,x,y,tol):
    lim = len(indices)

    j = 0
    for i in range(lim-1): # Actually, it works as a while, with a safety bound
        xj = x[j]
        yj = y[j]
        xm = x[j+1]
        ym = y[j+1]
        x2 = x[j+2]
        y2 = y[j+2]

        th1 = math.atan2( ym-yj, xm-xj )
        th2 = math.atan2( y2-ym, x2-xm )

        # The point (xm,ym) does not make a big change in direction : kill it
        if abs(th1-th2) < tol or abs(th1-th2-2*math.pi) < tol or\
          abs(th1-th2+2*math.pi) < tol : # this handles the case th1~pi and th2~-pi
            del(indices[j+1])
            del(x[j+1])
            del(y[j+1])
            j -= 1

        j += 1
        if j >= len(indices)-2:
            break

    return (indices,x,y)

# Computes a single polygon from a PNG
def singlePolygonFromPng(px,py,minlen,maxlen,ppi):
    npt = len(px)
    minlen2 = minlen**2
    maxlen2 = maxlen**2

    star    = np.argmax(px) # Choose the starting point
    polygon = [star] #Initialize the polygon

    # Now we do a loop
    pcur     = star
    pdir     = [1e-12,1]     # Previous direction
    d02      = 0             # This value will store the distance between first and second one

    for i in range(1000): #1000 is the limit in nb of iterations
        xc = px[pcur]
        yc = py[pcur]

        mine = npt # Initialize the current minimal angle value
        amin = 3*math.pi

        pool = [] # Contains all the points that will be removed for next step

        for j in range(npt): # looking for current one
            if j==pcur:
                continue # Next one cannot be current one

            x  = px[j]
            y  = py[j]
            d2 = (x-xc)**2 + (y-yc)**2

            if d2 > maxlen2:
                continue # Too far away

            pool.append(j)

            if j==star and d2 < d02 and len(polygon)>7: # We made a loop
                mine = star
                break

            if d2 < minlen2 and not (j==star):
                continue # Too close

            cdir = [x-xc,y-yc]
            vprod = pdir[0]*cdir[1] - pdir[1]*cdir[0] # Vectorial product
            sprod = pdir[0]*cdir[0] + pdir[1]*cdir[1] # Scalar product
            alpha = math.atan2( vprod, sprod )        # Angle

            if alpha < 0: # We need alpha to be positive
                alpha = 2*math.pi + alpha

            if alpha < amin:
                amin = alpha
                mine = j

        if mine == star: # Loop finished
            # TODO : find a way to be sure that the last point is not after the start point
            break

        if mine<npt: # Move forward
            x = px[mine]
            y = py[mine]
            if pcur == star: # Store the distance between first and second one (to avoid getting back)
                d02 = (yc-y)**2 + (xc-x)**2
            pcur = mine
            angl = math.atan2( yc-y, xc-x  )
            adir = [ xc-x, yc-y ]

            # Seek for the angle from witch we will start for next search
            # This angle should be such that previous iterates are not reachable
            anglj = 0
            for j in pool: #polygon

                xj = px[j]
                yj = py[j]
                d2 = (x-xj)**2 + (y-yj)**2

                if d2 > maxlen2:
                    continue # Not reachable anyway

                jdir = [ xj-x, yj-y ]

                vprod = adir[0]*jdir[1] - adir[1]*jdir[0] # Vectorial product
                sprod = adir[0]*jdir[0] + adir[1]*jdir[1] # Scalar product
                angla = math.atan2( vprod, sprod )        # Angle from adir to jdir
                if angla > math.pi/2: # Limit this angle
                    angla = 0
                anglj = max(anglj, angla)

            angl = angl + anglj + math.pi/1e4 # Slightly increment the angle
            #print(angl)

            pdir = [ math.cos(angl), math.sin(angl) ]
            polygon.append(mine)

        else: # Did not find any value
            print(('No more eligible point for polygon at sprite '+str(ppi)))
            break

    ppx = [ px[i] for i in polygon ]
    ppy = [ py[i] for i in polygon ]

    return(polygon,ppx,ppy)


# Computes polygons from points
def polygonFromPoints(points,minlen,maxlen):
    pxa = points[0]
    pya = points[1]

    npict = len(pxa)

    ppxs    = []
    ppys    = []
    polyall = []

    # List of values for minlen and maxlen. Both list should have len of llist
    minlist = [5,4,3,2,1]
    maxlist = [10,8,6,4,1.5]
    llist = 5

    # Adapt minlist and maxlist in order to match presripted values
    j = 0
    for i in range(llist):
        if minlist[j] > minlen or maxlist[j] > maxlen:
            del(minlist[j])
            del(maxlist[j])
            j -= 1
        j += 1
        if j >= len(minlist):
            break

    for ppi in range(npict):
        px = pxa[ppi]
        py = pya[ppi]

        for j in range(len(minlist)):
            stop = 1

            pplg    = singlePolygonFromPng(px,py,minlist[j],maxlist[j],ppi)
            polygon = pplg[0]
            ppx     = pplg[1]
            ppy     = pplg[2]

            # Some checks
            if len(polygon)==1001:
                print('refining sprite '+str(ppi))
                stop = 0

            elif abs(max(ppx)-max(px)) > minlen or abs(min(ppx)-min(px)) > minlen \
              or abs(max(ppy)-max(py)) > minlen or abs(min(ppy)-min(py)) > minlen:
                print('Polygon is not precise enough. Refining sprite '+str(ppi))
                stop = 0

            if stop:
                break

        polysim = simplifyPolygon(polygon,ppx,ppy,math.pi/16) # Simplify the polygon

        ppxs.append(polysim[1])
        ppys.append(polysim[2])
        polyall.append(polysim[0])

    return (polyall,ppxs,ppys)

# Computes a polygon from PNG
def polygonFromPng(address,sx,sy,ceil,minlen,maxlen):
    points  = pointsFromPng(address,sx,sy,ceil)
    polygon = polygonFromPoints(points,minlen,maxlen)
    return (points,polygon)

# Generates a XML file that contains the polygon
def generateXML(polygon,address):

    poly = polygon[0]
    px   = polygon[1]
    py   = polygon[2]

    nb   = len(poly)

    polygons = ET.Element('polygons')

    for i in range(nb):
        ppx   = px[i]
        ppy   = py[i]
        npoly = len(ppx)

        polyg = ET.SubElement(polygons,'polygon')
        x = ET.SubElement(polyg,'x')
        y = ET.SubElement(polyg,'y')

        strx = str(ppx[0])
        stry = str(ppy[0])
        for j in range(npoly-1):
            strx = strx + ',' + str(ppx[j+1])
            stry = stry + ',' + str(ppy[j+1])

        x.text = strx #strx
        y.text = stry

    mydata = ET.tostring(polygons, encoding="UTF-8", method="xml")
    mydata = pretty.parseString(mydata)
    mydata = mydata.toprettyxml(indent="\t",encoding="UTF-8")

    myfile = open(address, "w")
    myfile.write(mydata.decode("utf-8") )
    myfile.close()

# Generates polygon for all outfits
def polygonify_single(fileName, polyAddress, sx=1, sy=1, ceil=150, minlen=3, maxlen=6, overwrite=False ):
    # Test if the file already exists
    if ( not overwrite and os.path.exists(polyAddress) ) :
        return

    print("Generation of " + polyAddress)

    pntNplg = polygonFromPng( fileName, sx, sy, ceil, minlen, maxlen )

    polygon = pntNplg[1]
    generateXML(polygon,polyAddress)

# Generates polygon for all outfits
def polygonify_all_outfits(gfxPath, polyPath, overwrite):

    # Default parameters
    default_maxNmin = (2,4)

    # First define the parameters for special files
    maxNmin = {
               "ripperM" : (3,6)
              }

    for fileName in os.listdir(gfxPath):
        if (fileName.endswith((".png", ".webp")) and not fileName.endswith(("-end.png", "-end.webp"))) \
           and not fileName.startswith("beam_"):

            polyAddress = (polyPath+fileName+".xml")

            # Test if the file already exists
            if ( not overwrite and os.path.exists(polyAddress) ) :
                continue

            # Special case because of strange non-connex pixels
            if fileName == 'autocannon.png':
                print('Warning : autocannon.png was not generated')
                continue

            # Manage parameters
            lmin = default_maxNmin[0]
            lmax = default_maxNmin[1]
            if fileName in maxNmin:
                mNm = maxNmin[fileName]
                lmin = mNm[0]
                lmax = mNm[1]

            pngAddress  = (gfxPath+fileName)

            print("Generation of " + polyAddress)

            pntNplg = polygonFromPng(pngAddress,6,6,150,lmin,lmax)

            polygon = pntNplg[1]
            generateXML(polygon,polyAddress)

# Generates polygon for all asteroids
def polygonify_all_asteroids( gfxPath, polyPath, overwrite ):

    # Default parameters
    default_maxNmin = (3,6,150)

    # First define the parameters for special files
    maxNmin = { "flower01" : (5,10,150) } # Actually, the algorithm automatically refines this one, so it could be skipped

    for fileName in os.listdir(gfxPath):

        polyAddress = (polyPath+fileName+".xml")

        # Test if the file already exists
        if ( not overwrite and os.path.exists(polyAddress) ) :
            continue

        # Manage parameters
        lmin = default_maxNmin[0]
        lmax = default_maxNmin[1]
        ceil = default_maxNmin[2]
        if fileName in maxNmin:
            mNm = maxNmin[fileName]
            lmin = mNm[0]
            lmax = mNm[1]
            ceil = mNm[2]

        pngAddress  = (gfxPath+fileName)

        print("Generation of " + polyAddress)

        pntNplg = polygonFromPng(pngAddress,1,1,ceil,lmin,lmax)
        polygon = pntNplg[1]

#        points  = pntNplg[0]
#        plt.figure()
#        plt.title(polyAddress)
#        plt.scatter(points[0][0],points[1][0])
#        plt.scatter(polygon[1][0],polygon[2][0])

        generateXML(polygon,polyAddress)

# Generates polygon for all ships
def polygonify_all_ships( gfxPath, polyPath, overwrite ):

    # Default parameters
    default_maxNmin = (8,8,150,3,6)

    # First define the parameters for special files
    maxNmin = {
               "apprehension" : (12,12,150,3,6),
               "archimedes" : (12,12,150,3,6),
               "arx" : (12,12,150,3,6),
               "arx_feral" : (12,12,150,3,6),
               "certitude" : (12,12,150,3,6),
               "demon" : (12,12,150,3,6),
               "diablo" : (12,12,1,4,8),
               "divinity" : (12,12,150,3,6),
               "dogma" : (12,12,150,3,6),
               "drone_heavy" : (10,10,150,3,6),
               "drone_carrier": (12,12,150,3,6),
               "goddard" : (12,12,150,3,6),
               "goddard_dvaered" : (12,12,150,3,6),
               "hawking" : (12,12,150,3,6),
               "hawking_empire" : (12,12,150,3,6),
               "hephaestus" : (12,12,1,4,8),
               "imp" : (12,12,150,3,6),
               "ira" : (12,12,150,3,6),
               "kahan" : (10,10,150,3,6),
               "kestrel" : (10,10,150,3,6),
               "kestrel_pirate" : (10,10,150,3,6),
               "mephisto" : (12,12,150,3,6),
               "mule" : (10,10,150,3,6),
               "nyx" : (10,10,150,3,6),
               "pacifier" : (10,10,150,3,6),
               "pacifier_empire" : (10,10,150,3,6),
               "peacemaker" : (12,12,150,3,6),
               "phalanx" : (10,10,150,3,6),
               "phalanx_pirate" : (10,10,150,3,6),
               "phalanx_dvaered" : (10,10,150,3,6),
               "preacher" : (10,10,150,3,6),
               "prototype" : (12,12,150,3,6),
               "quicksilver" : (10,10,150,3,6),
               "rhino" : (10,10,150,3,6),
               "rhino_pirate" : (10,10,150,3,6),
               "sting" : (10,10,1,3,6),
               "taciturnity" : (10,10,150,3,6),
               "vigilance" : (10,10,150,3,6),
               "vigilance_dvaered" : (10,10,150,3,6),
               "vox" : (12,12,150,4,8),
               "watson" : (12,12,150,3,6),
               "apparition_fighter": (8,8,150,3,6),
               "apparition_corvette": (10,10,150,3,6),
               "apparition_cruiser": (12,12,150,3,6),
               "mammon_zalek": (12,12,150,3,6),
               "arsenal_dvaered": (12,12,150,3,6),
               "retribution_dvaered": (12,12,150,3,6),
               "rainmaker_empire": (12,12,150,3,6),
               "copia": (12,12,150,3,6),
               "providence": (12,12,150,3,6),
               "zebra": (12,12,150,3,6),
               "zebra_pirate": (12,12,150,3,6),
               "pythagoras": (12,12,150,3,6),
               "gauss": (10,10,150,3,6),
               "starbridge": (10,10,150,3,6),
               "starbridge_pirate": (10,10,150,3,6),
               "hippocrates": (10,10,150,3,6),
              }

    def goodfile( filename ):
        if (fileName.endswith(".png") and \
            not fileName.endswith("_comm.png")) \
            and not fileName.endswith("_engine.png"):
           return True
        if (fileName.endswith(".webp") and \
            not fileName.endswith("_comm.webp")) \
            and not fileName.endswith("_engine.webp"):
           return True
        return False

    for root, directories, filenames in os.walk(gfxPath):
        for fileName in filenames:
            if goodfile( fileName ):

                # Remove the .png
                name = os.path.splitext(fileName)[0]

                polyAddress = (polyPath+name+".xml")

                # Test if the file already exists
                if ( not overwrite and os.path.exists(polyAddress) ) :
                    continue

                # Manage parameters
                sx   = default_maxNmin[0]
                sy   = default_maxNmin[1]
                ceil = default_maxNmin[2]
                lmin = default_maxNmin[3]
                lmax = default_maxNmin[4]

                basefileName = fileName
                if basefileName[-4:] == '.png':
                    basefileName = basefileName[:-4]
                if basefileName[-5:] == '.webp':
                    basefileName = basefileName[:-5]
                if basefileName in maxNmin:
                    mNm = maxNmin[basefileName]
                    sx   = mNm[0]
                    sy   = mNm[1]
                    ceil = mNm[2]
                    lmin = mNm[3]
                    lmax = mNm[4]

                pngAddress  = (root+"/"+fileName)

                print("Generation of " + polyAddress + ". Parameters are : ("\
                       + str(sx) + ", " + str(sy) + ", " + str(ceil) + ", "\
                       + str(lmin) + ", " + str(lmax) + ")")

                pntNplg = polygonFromPng(pngAddress,sx,sy,ceil,lmin,lmax)

                polygon = pntNplg[1]
                generateXML(polygon,polyAddress)

# Run stuff
if __name__ == "__main__":
    basepath = "artwork/"
    overwrite = True

    # Special cases where we use spob assets for ships
    polygonify_single( basepath+'gfx/spob/space/000.webp', basepath+'gfx/ship_polygon/000.webp.xml', overwrite=overwrite )
    polygonify_single( basepath+'gfx/spob/space/002.webp', basepath+'gfx/ship_polygon/002.webp.xml', overwrite=overwrite )
    polygonify_single( basepath+'gfx/spob/space/station-battlestation.webp', basepath+'gfx/ship_polygon/station-battlestation.webp.xml', overwrite=overwrite )
    polygonify_single( basepath+'gfx/spob/space/derelict_goddard.webp', basepath+'gfx/ship_polygon/derelict_goddard.webp.xml', minlen=1, overwrite=overwrite )
    # All ships
    polygonify_all_ships( basepath+'gfx/ship/', basepath+'gfx/ship_polygon/', overwrite=overwrite )
    # All outfits
    polygonify_all_outfits( basepath+'gfx/outfit/space/', basepath+'gfx/outfit/space_polygon/', overwrite=overwrite )
    # All Asteroids
    polygonify_all_asteroids( basepath+'gfx/spob/space/asteroid/', basepath+'gfx/spob/space/asteroid_polygon/', overwrite=overwrite )

    # Use the above stuff to generate only one ship or outfit polygon :

    #pntNplg = polygonFromPng('../../../naev-artwork-production/gfx/spob/space/asteroid/asteroid-D51.webp',1,1,150,3,6)
    #pntNplg = polygonFromPng('../../../naev-artwork-production/gfx/spob/space/asteroid/asteroid-D51_bis.png',1,1,150,3,6)
    #pntNplg = polygonFromPng('../naev/dat/gfx/ship/shark/shark.png',8,8,150,3,6)
    #pntNplg = polygonFromPng('../../../naev/dat/gfx/outfit/space/caesar.png',6,6,1,2,4)

#    points  = pntNplg[0]
#    polygon = pntNplg[1]
#
#    poly = polygon[0]
#
#    plt.scatter(points[0][0],points[1][0])
#    plt.scatter(polygon[1][0],polygon[2][0])

    #generateXML(polygon,'caesar.xml')
