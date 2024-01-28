#!/usr/bin/env python3

"""
Generates collision polygons from image or gltf files

WARNING : this script uses python 3
For needed depandancies, please see the import section

If you added a space ammo/bolt png:
Go to bottom of the file, and replace '../naev/dat/gfx/outfit/space/' and
'../naev/dat/gfx/outfit/space_polygon/' by the right paths in naev repo
Then, commertarize the line "polygonify_all_ships" and run this script
from shell (or any other interface that prints warnings).
If the script refines the polygon a lot (more than 3-4 times), there
is maybe something wrong. You'll have to add your file to special files
in "polygonify_all_outfits" function. To see what parameters to put,
read the rest of this text, or request help on discord.

If you added a ship png:
Go to bottom of the file, and replace '../naev/dat/gfx/ship/' and
'../naev/dat/gfx/ship_polygon/' by the right paths in naev repo
Then, commentarize the line "polygonify_all_outfits".
If your png has more than 8X8 sprites,
go in the function "polygonify_all_ships" and add the name of your
png in the dictionnary maxNmin, along with (sx,sy,50,3,6).
If your sprite is very big (2048), replace 3,6 by 4,8
Then,run the script from shell
(or any other interface that prints warnings).
If the script refines the polygon a lot (more than 10-15 times), there
is maybe something wrong. To see what parameters to use,
read the rest of this text, or request help on discord.

Generating data for all outfits :
polygonify_all_outfits( origin_address, destination_address, overwrite )

Generating data for all ships :
polygonify_all_ships( origin_address, destination_address, overwrite )

If the third argument (overwrite) is 0, only new file will be generated
if overwrite == 1, all the polygons will be generated

Using the algorithm for one png :

1)Run polygonFromImg('address_of_my_png',sx,sy,50,3,6)
sx and sy are the nb of sprites in the picture (ex for Lancelot, its 8 and 8)
If the code doesn't give any warning, run generateXML and put the generated
file in ship_polygon or space_polygon directory.
The last two arguments are the min and max length of the polygon's faces.
Usually, I use (3,6) for ships and (2,4) for outfits.

2)If the set of points is not connex, while the png is, decrease the alpha_threshold
(4th argument of polygonFromImg, down to 1 if needed)

3)If the code says that sx or sy are wrong, check that the values you have
given are right. If they are, as written in the warning message, the error
will not be greater as 1 pixel.

4)For extremely big ships, like the diablo, it could be interesting to use
coarser dimensions like 4 and 8 for the last two arguments

5)For purposely non-connex objects (like the ripper shot), the aim is to
build a polygon that contains all the parts of the object. For that purpose,
increase the last 2 arguments up to (4,8), or even (5,10).
Remark: for these cases, a convex polygon algorithm could be preferable...

/!\ Very important : once you've found the right parameters to generate the
polygon from your png, add them in the dictionnary maxNmin
(except if the default values are right)


Principle of the algorithm :

1 ) The transparency array is transformed into a set of points in
PointsFromImg.
Each point is adjacent to 4 pixels. In order for that point
to be activated, at least 1 of the 4 pixels must have an opacity value that
is greater than the alpha_threshold (50 is a good value for most ships)

Rem 1 : If all the pixels have opacity>=alpha_threshold, the point is not activated as
this point is for sure not on the boundary and only boundary are important
in what we want to do

Rem 2 : The value of alpha_threshold of 50 is totally arbitrary

2 ) The set of points is transformed into a polygon in polygonFromImg.
The algo picks up one of the rightmost points. This point is the starting
point. We define as well the starting direction as nearly vertical
  Then the following recurcive algo runs :
  a/ From the current point, find the next point that is at a distance
     between minlen and maxlen and that makes the minimal angle with the
     previous direction.
  b/ Compute the next direction in order to prevent going backwards.
     This direction makes the maximal angle with the direction going from
     current point to previous point, and is chosen among the points that
     are at a distance < maxlen from the previous point.
     This step is necessary in the concave parts of ships.
  After that, a few checks are performed. If the generated polygon fails,
  a new finer polygon is generated.

3 ) The polygon is simplified in simplifyPolygon.
A loop is run among the points of the polygon. Any point which angle is too
close to pi is suppressed.
"""

import numpy as np
import math
import matplotlib.pyplot as plt
import xml.etree.ElementTree as ET
import xml.dom.minidom as pretty
#from pygltflib import GLTF2
#import struct
from stl import mesh
import os
import sys
import argparse
import tempfile
import subprocess
from tqdm import tqdm

# Create an array from an image
def arrFromImg( address, sx, sy ):
    buffer = plt.imread( address )

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
def pointsFromImg( address, sx, sy, alpha_threshold ):
    if type(address)==str:
        picture = arrFromImg( address, sx, sy )
    else:
        picture = address
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
        # to corner is > alpha_threshold, put a point at this corner
        # + + + + + + + +
        #  0 1 1 1 0 0 0
        # + + + + + + + +
        #  0 0 1 1 1 0 0
        # + + + + + + + +

        for i in range(ssx+1): # There is 1 corner more than pixels
            for j in range(ssy+1):
                if i==0:
                    if j==0:
                        if pictcur[0,0] >= alpha_threshold:
                            bufferx.append(-i+sx2) # -i because image vs coordinates
                            buffery.append(j-sy2)
                    elif j==ssy:
                        if pictcur[0,ssy-1] >= alpha_threshold:
                            bufferx.append(-i+sx2)
                            buffery.append(j-sy2)
                    else:
                        if pictcur[0,j-1] >= alpha_threshold or pictcur[0,j] >= alpha_threshold:
                            bufferx.append(-i+sx2)
                            buffery.append(j-sy2)

                elif i==ssx:
                    if j==0:
                        if pictcur[ssx-1,0] >= alpha_threshold:
                            bufferx.append(-i+sx2)
                            buffery.append(j-sy2)
                    elif j==ssy:
                        if pictcur[ssx-1,ssy-1] >= alpha_threshold:
                            bufferx.append(-i+sx2)
                            buffery.append(j-sy2)
                    else:
                        if pictcur[ssx-1,j-1] >= alpha_threshold or pictcur[ssx-1,j] >= alpha_threshold:
                            bufferx.append(-i+sx2)
                            buffery.append(j-sy2)

                else:
                    if j==0:
                        if pictcur[i-1,0] >= alpha_threshold or pictcur[i,0] >= alpha_threshold:
                            bufferx.append(-i+sx2)
                            buffery.append(j-sy2)
                    elif j==ssy:
                        if pictcur[i-1,ssy-1] >= alpha_threshold or pictcur[i,ssy-1] >= alpha_threshold:
                            bufferx.append(-i+sx2)
                            buffery.append(j-sy2)
                    else:
                        # This is the most general case. Remove points inside the domain
                        if (pictcur[i-1,j-1] >= alpha_threshold or pictcur[i-1,j] >= alpha_threshold \
                            or pictcur[i,j-1] >= alpha_threshold or pictcur[i,j] >= alpha_threshold) \
                            and not (pictcur[i-1,j-1] >= alpha_threshold and pictcur[i-1,j] >= alpha_threshold \
                            and pictcur[i,j-1] >= alpha_threshold and pictcur[i,j] >= alpha_threshold):
                            bufferx.append(-i+sx2)
                            buffery.append(j-sy2)

        pointsx.append(buffery)
        pointsy.append(bufferx) # We invert because pictures and matrix dont use the same coordinate system

    return (pointsx,pointsy)

# Simplify a polygon by removing points that are aligned with other points
def simplifyPolygon( indices, x, y, tol ):
    lim = len(indices)
    if lim < 3:
        return (indices, x, y)

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

    return (indices, x, y)

# Create the projections of the ship from the STL data
def pointsFrom3D( address, slices, size, center, alpha ):
    # Note doing it directly from GLTF is not working, so we use blender to drop to STL
    """
    # Extract the mesh and the points
    gltf = GLTF2().load( address )
    for s in gltf.scenes:
        if s.name=="body":
            scene = s
            break
    vertices = []
    triangles = []
    for meshid in scene.nodes:
        mesh = gltf.meshes[ meshid ]
        for primitive in mesh.primitives:
            # get the binary data for this mesh primitive from the buffer
            accessor = gltf.accessors[ primitive.attributes.POSITION ]
            bufferView = gltf.bufferViews[ accessor.bufferView ]
            buffer = gltf.buffers[ bufferView.buffer ]
            data = gltf.get_data_from_buffer_uri( buffer.uri )

            # pull each vertex from the binary buffer and convert it into a tuple of python floats
            for i in range(accessor.count):
                index = bufferView.byteOffset + accessor.byteOffset + i*12  # the location in the buffer of this vertex
                d = data[index:index+12]  # the vertex data
                v = struct.unpack("<fff", d)   # convert from base64 to three floats
                vertices.append(v)

            # unpack floats
            vertices2 = []
            for a,b,c in vertices:
                vertices2 += [a,b,c]

            # create triangles
            vertices = vertices2
            # TODO not sure why there's not a good amount of triangles...
            for i in range(0,math.floor(len(vertices)/9)*9,9):
                triangles.append(vertices[i:i+9])

    triangles = np.array(triangles).transpose()
    v0 = np.array(triangles[:][0:3])
    v1 = np.array(triangles[:][3:6])
    v2 = np.array(triangles[:][6:9])
    """

    stlfile = tempfile.NamedTemporaryFile( suffix=".stl" )
    gltftostlpy = os.path.dirname(sys.argv[0])+"/blend_gltf_to_stl.py"
    ret = subprocess.run(['blender', '--background', '--python', gltftostlpy, '--', address, stlfile.name ])
    if ret.returncode != 0:
        print("Warning: GLTF to STL export failed.")

    shipMesh = mesh.Mesh.from_file( stlfile.name )
    v0 = np.transpose(shipMesh.v0) # TODO : take the center into account
    v1 = np.transpose(shipMesh.v1)
    v2 = np.transpose(shipMesh.v2)

    # Compute x and y max and min
    xM = max( [np.amax(v0[0,:]), np.amax(v1[0,:]), np.amax(v2[0,:])] )
    xm = min( [np.amin(v0[0,:]), np.amin(v1[0,:]), np.amin(v2[0,:])] )
    yM = max( [np.amax(v0[1,:]), np.amax(v1[1,:]), np.amax(v2[1,:])] )
    ym = min( [np.amin(v0[1,:]), np.amin(v1[1,:]), np.amin(v2[1,:])] )

    # Rescale the data
    leng   = max(xM-xm,yM-ym)
    factor = size/leng

    v0 = factor*v0
    v1 = factor*v1
    v2 = factor*v2

    # Rotate the stuff for any angle
    dtheta = 2*math.pi/slices

    xlist = []
    ylist = []

    for it in tqdm(range(slices), desc="Transforming model", ascii=True):
        # Rotate the points
        theta = it*dtheta + math.pi/2
        rot = np.matrix([[math.cos(theta), -math.sin(theta), 0],\
                         [math.sin(theta), math.cos(theta), 0],\
                         [0, 0, 1]])
        vt0 = rot * v0
        vt1 = rot * v1
        vt2 = rot * v2

        # Projection for the view
        proj = np.matrix([[1, 0, 0],\
                          [0, 1, math.tan(alpha)],\
                          [0, 0, 0]])
        vt0 = proj * vt0
        vt1 = proj * vt1
        vt2 = proj * vt2

        # Extract x and y coordinates
        x0 = vt0[0,:]
        x1 = vt1[0,:]
        x2 = vt2[0,:]
        y0 = vt0[1,:]
        y1 = vt1[1,:]
        y2 = vt2[1,:]

        xmax = max( [np.amax(x0), np.amax(x1), np.amax(x2)] )
        xmin = min( [np.amin(x0), np.amin(x1), np.amin(x2)] )
        ymax = max( [np.amax(y0), np.amax(y1), np.amax(y2)] )
        ymin = min( [np.amin(y0), np.amin(y1), np.amin(y2)] )

        # Now we create a grid of points that are inside the ship.
        # We need this regular grid because its the only way to have a
        # non-convex polygon generation algo that is guaranteed to work.
        xgrid = []
        ygrid = []
        fullDots = np.zeros( ( int(xmax)-int(xmin)+1 , int(ymax)-int(ymin)+1 ) )

        for ai, i in enumerate( range(int(xmin),int(xmax)+1) ):
            for aj, j in enumerate( range(int(ymin),int(ymax)+1) ):
                # Test if there is a triangle for which the point (i,j) is inside
                # Here, we do vector operations to speed up computations
                D1 = np.multiply(x1-i,y2-j) - np.multiply(x2-i,y1-j);
                D2 = np.multiply(x2-i,y0-j) - np.multiply(x0-i,y2-j);
                D3 = np.multiply(x0-i,y1-j) - np.multiply(x1-i,y0-j);
                D0 = D1+D2+D3;

                j1 = np.where(np.multiply(D0,D1) > 0)[1];
                j2 = np.where(np.multiply(D0,D2) > 0)[1];
                j3 = np.where(np.multiply(D0,D3) > 0)[1];
                j4 = np.intersect1d( np.intersect1d(j1,j2), j3);

                # Hell, there are flat triangles. As a consequence, > cannot
                # be replaced by >= in np.where
                if len(j4) >= 1: # point is in a triangle
                    fullDots[ai,aj] = 1

        # Second loop to remove points that are inside the domain
        # (to speedup polygon generation)
        for ai, i in enumerate( range(int(xmin),int(xmax)+1) ):
            for aj, j in enumerate( range(int(ymin),int(ymax)+1) ):
                if fullDots[ai,aj] == 1:
                    if (i == int(xmin) or i == int(xmax) or \
                       j == int(ymin) or j == int(ymax)):
                           # We're on the boundary : activate the point
                           xgrid.append(i)
                           ygrid.append(j)
                    elif (fullDots[ai-1,aj] == 1 and fullDots[ai,aj-1] == 1\
                       and fullDots[ai+1,aj] == 1 and fullDots[ai,aj+1] == 1) :
                           # This point is inside the shape. Don't activate it
                           pass
                    else:
                        xgrid.append(i)
                        ygrid.append(j)

        xlist.append(xgrid)
        ylist.append(ygrid)

        #plt.scatter(xgrid,ygrid)
        #plt.scatter(x0.tolist()[0],y0.tolist()[0])
        #break

    return (xlist, ylist, factor)

# Computes a single polygon from an image
def singlePolygonFromImg( px, py, minlen, maxlen, ppi ):
    npt = len(px)
    minlen2 = minlen**2
    maxlen2 = maxlen**2

    star    = np.argmax(px) # Choose the starting point
    polygon = [star] # Initialize the polygon

    # Now we do a loop
    pcur     = star
    pdir     = [1e-12,1]     # Previous direction
    d02      = 0             # This value will store the distance between first and second one

    for i in range(1000): # Limit number of iterations
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

        if mine < npt: # Move forward
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
            print('No more eligible point for polygon at sprite '+str(ppi))
            break

    ppx = [ px[i] for i in polygon ]
    ppy = [ py[i] for i in polygon ]

    return(polygon, ppx, ppy)


# Computes polygons from points
def polygonFromPoints( points, minlen, maxlen ):
    pxa = points[0]
    pya = points[1]

    npict = len(pxa)

    ppxs    = []
    ppys    = []
    polyall = []

    # List of values for minlen and maxlen. Both list should have same length
    minlist = [ 5,  4, 3, 2, 1 ]
    maxlist = [ 10, 8, 6, 4, 1.5 ]
    assert( len(minlist)==len(maxlist) )

    # Adapt minlist and maxlist in order to match presripted values
    minlist = list(filter(lambda x: x <= minlen, minlist))
    maxlist = list(filter(lambda x: x <= maxlen, maxlist))

    for ppi in range(npict):
        px = pxa[ppi]
        py = pya[ppi]

        for j in range(len(minlist)):
            stop = 1

            pplg    = singlePolygonFromImg( px, py, minlist[j], maxlist[j], ppi )
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

        polysim = simplifyPolygon( polygon, ppx, ppy, math.pi/16 ) # Simplify the polygon

        ppxs.append( polysim[1] )
        ppys.append( polysim[2] )
        polyall.append( polysim[0] )

    return (polyall, ppxs, ppys)

# Computes a polygon from an image
def polygonFromImg( address, sx, sy, alpha_threshold, minlen, maxlen ):
    points  = pointsFromImg( address, sx, sy, alpha_threshold )
    polygon = polygonFromPoints( points, minlen, maxlen )
    return (points, polygon)

# Computes a polygon from an STL
def polygonFrom3D( address, slices=72, scale=30, center=[0,0,0], alpha=math.pi/4, minlen=3, maxlen=6 ):
    points  = pointsFrom3D( address, slices, scale, center, alpha )
    xlist   = points[0]
    ylist   = points[1]
    factor  = points[2]
    polygon = polygonFromPoints( (xlist, ylist), minlen, maxlen )

    # Rescale by dividing by factor
    xlist = polygon[1]
    ylist = polygon[2]
    xlist = [np.array(i)/factor for i in xlist]
    ylist = [np.array(i)/factor for i in ylist]

    xpoint = points[0]
    ypoint = points[1]
    xpoint = [np.array(i)/factor for i in xpoint]
    ypoint = [np.array(i)/factor for i in ypoint]

    return ( (xpoint,ypoint), (polygon[0],xlist,ylist) )

# Generates a XML file that contains the polygon
def generateXML( polygon, address ):

    os.makedirs( os.path.dirname(address), exist_ok=True )

    poly = polygon[0]
    px   = polygon[1]
    py   = polygon[2]

    nb   = len(poly)

    polygons = ET.Element('polygons')
    polygons.set("num", f"{nb}")
    for i in range(nb):
        polyg = ET.SubElement(polygons,'polygon')
        polyg.set("num", f"{len(px[i])}" )
        x = ET.SubElement(polyg,'x')
        y = ET.SubElement(polyg,'y')
        x.text = ",".join(map(lambda x: str(x), px[i]))
        y.text = ",".join(map(lambda x: str(x), py[i]))

    mydata = ET.tostring(polygons, encoding="UTF-8", method="xml")
    mydata = pretty.parseString(mydata)
    mydata = mydata.toprettyxml(indent="\t",encoding="UTF-8")

    myfile = open(address, "w")
    myfile.write(mydata.decode("utf-8") )
    myfile.close()

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

            pntNplg = polygonFromImg( pngAddress, 6, 6, 50, lmin, lmax )

            polygon = pntNplg[1]
            generateXML( polygon, polyAddress )

# Generates polygon for all asteroids
def polygonify_all_asteroids( gfxPath, polyPath, overwrite ):

    # Default parameters
    default_maxNmin = (3,6,50)

    # First define the parameters for special files
    maxNmin = { "flower01" : (5,10,50) } # Actually, the algorithm automatically refines this one, so it could be skipped

    for fileName in os.listdir(gfxPath):

        polyAddress = (polyPath+fileName+".xml")

        # Test if the file already exists
        if ( not overwrite and os.path.exists(polyAddress) ) :
            continue

        # Manage parameters
        lmin = default_maxNmin[0]
        lmax = default_maxNmin[1]
        alpha_threshold = default_maxNmin[2]
        if fileName in maxNmin:
            mNm = maxNmin[fileName]
            lmin = mNm[0]
            lmax = mNm[1]
            alpha_threshold = mNm[2]

        pngAddress  = (gfxPath+fileName)

        print("Generation of " + polyAddress)

        pntNplg = polygonFromImg( pngAddress, 1, 1, alpha_threshold, lmin, lmax )
        polygon = pntNplg[1]

        """
        points  = pntNplg[0]
        plt.figure()
        plt.title(polyAddress)
        plt.scatter(points[0][0],points[1][0])
        plt.scatter(polygon[1][0],polygon[2][0])
        """

        generateXML(polygon,polyAddress)


def polygonify_ship( filename, outpath, use_3d=True ):
    root = ET.parse( filename ).getroot()
    name = root.get('name')
    cls = root.find( "class" ).text
    tag = root.find( "GFX" )
    if tag != None:
        outname = f"{outpath}/ship/{tag.text}.xml"

        if use_3d:
            # Try 3D first
            try:
                gltfpath = f"artwork/gfx/ship3d/{tag.text.split('_')[0]}/{tag.text}.gltf"
                gltfpath = os.getenv("HOME")+f"/.local/share/naev/plugins/3dtest/gfx/ship3d/{tag.text.split('_')[0]}/{tag.text}.gltf"
                pntNplg = polygonFrom3D( gltfpath, scale=int(tag.get("size")) )
            except:
                use_3d = False
        # Fall back to image
        if not use_3d:
            print("Failed to find 3D model, falling back to 2D")
            imgpath = f"artwork/gfx/ship/{tag.text.split('_')[0]}/{tag.text}.webp"
            if not os.path.isfile(imgpath):
                imgpath = f"artwork/gfx/ship/{tag.text.split('_')[0]}/{tag.text}.png"
            try:
                sx = int(tag.get("sx"))
            except:
                sx = 8
            try:
                sy = int(tag.get("sy"))
            except:
                sy = 8
            alpha_threshold    = 50
            minlen  = 3
            maxlen  = 6
            img     = arrFromImg( imgpath, sx, sy )
            if img[0].shape[0] > 50:
                minlen = 4
                maxlen = 8
            pntNplg = polygonFromImg( img, sx, sy, alpha_threshold, minlen, maxlen )

        # Now Generate the
        polygon = pntNplg[1]
        generateXML( polygon, outname )

        """
        points = pntNplg[0]
        plt.figure()
        plt.title( outname )
        plt.scatter(points[0][0],points[1][0])
        plt.scatter(polygon[1][0],polygon[2][0])
        plt.show()
        """

# Run stuff
if __name__ == "__main__":
    parser = argparse.ArgumentParser( description='Wrapper for luacheck that "understands" Naev hooks.' )
    parser.add_argument('path', metavar='PATH', nargs='+', type=str, help='Name of the path(s) to parse. Recurses over .lua files in the case of directories.')
    parser.add_argument('--outpath', type=str, default="dat/polygon" )
    parser.add_argument("--use_3d", type=bool, default=True )
    args, unknown = parser.parse_known_args()

    for a in args.path:
        print(f"Processing {a}...")
        polygonify_ship( a, args.outpath, args.use_3d )

    """
    # Special cases where we use spob assets for ships
    polygonify_single( inpath, 'spob/space/000.webp', outpath, overwrite=overwrite )
    polygonify_single( inpath, 'spob/space/002.webp', outpath, overwrite=overwrite )
    polygonify_single( inpath, 'spob/space/station-battlestation.webp', outpath, overwrite=overwrite )
    polygonify_single( inpath, 'spob/space/derelict_goddard.webp', outpath, minlen=1, overwrite=overwrite )
    # All ships
    polygonify_all_ships( inpath, 'ship/', outpath, overwrite=overwrite )
    # All outfits
    polygonify_all_outfits( inpath, 'outfit/space/', outpath+'gfx/outfit/space_polygon/', overwrite=overwrite )
    # All Asteroids
    polygonify_all_asteroids( inpath, 'spob/space/asteroid/', outpath+'gfx/spob/space/asteroid_polygon/', overwrite=overwrite )

    # Use the above stuff to generate only one ship or outfit polygon :

    #pntNplg = polygonFromImg('../../../naev-artwork-production/gfx/spob/space/asteroid/asteroid-D51.webp',1,1,50,3,6)
    #pntNplg = polygonFromImg('../../../naev-artwork-production/gfx/spob/space/asteroid/asteroid-D51_bis.png',1,1,50,3,6)
    #pntNplg = polygonFromImg('../naev/dat/gfx/ship/shark/shark.png',8,8,50,3,6)
    #pntNplg = polygonFromImg('../../../naev/dat/gfx/outfit/space/caesar.png',6,6,1,2,4)

#    points  = pntNplg[0]
#    polygon = pntNplg[1]
#
#    poly = polygon[0]
#
#    plt.scatter(points[0][0],points[1][0])
#    plt.scatter(polygon[1][0],polygon[2][0])

    #generateXML(polygon,'caesar.xml')
    """
