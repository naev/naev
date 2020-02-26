# Generates polygons from png files

import numpy as np
import math

from scipy import ndimage
from PIL import Image
import matplotlib.pyplot as plt

import xml.etree.ElementTree as ET
import xml.dom.minidom as pretty
#root = ET.parse('/home/renaud/mes_programmes/naev/dat/ships/admonisher.xml').getroot()
#root = ET.parse('/home/renaud/mes_programmes/naev/dat/ships/soromid_arx.xml').getroot()
#a = root.findall('GFX')
#root.get('name')
#a.get('sx')
#rom matplotlib import pyplot as plt

# Create an array from the picture
def arrFromPng(adress,sx,sy):
    picture = ndimage.imread(adress)
    
    if np.shape(picture)[2] == 4:
        picture = picture[:,:,3]
    elif np.shape(picture)[2] == 2: # Black and white
        picture = picture[:,:,1]
    else:
        print('Warning: unable to read png file')
        picture = picture[:,:,3] # Try this, maybe it will work
    
    # Store all the different pictures as a tensor
    six = picture.shape[0]/sx
    siy = picture.shape[1]/sy
    
    if (int(six) != six) or (int(siy) != siy) :
        print('Warning: sx or sy is wrong')
        
    six = int(six)
    siy = int(siy)

    pictensor = np.zeros(shape = (six,siy,sx*sy), dtype = np.uint8)
    for i in range(sx):
        for j in range(sy):
            sxy = sy*i + j
            pictensor[:,:,sxy] = picture[ six*i:six*(i+1), siy*j:siy*(j+1) ]
            
    return pictensor

# Defines points from png
def pointsFromPng(adress,sx,sy,ceil):
    picture = arrFromPng(adress,sx,sy)
    
    ssx   = picture.shape[0]
    ssy   = picture.shape[1]
    npict = picture.shape[2]
    #print(npict)
    sx2 = ssx/2 # Offset value
    sy2 = ssy/2

    pointsx = [] # This list of list will contian the abscissae of the points
    pointsy = []
    
    for p in range(npict):
        #print(p)
        pictcur = picture[:,:,p]
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
                    #print(j)
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
                            
        #print(len(bufferx))
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

# Computes a polygon from PNG
def polygonFromPng(adress,sx,sy,ceil,minlen,maxlen):
    points = pointsFromPng(adress,sx,sy,ceil)
    
    pxa = points[0]
    pya = points[1]
    
    npict = len(pxa)
    
    ppxs    = []
    ppys    = []
    polyall = []
    
    for ppi in range(npict):#[16]:#range(npict):
        px = pxa[ppi]
        py = pya[ppi]
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
                    #print(j)
                    #print(d2)
                    vprod = adir[0]*jdir[1] - adir[1]*jdir[0] # Vectorial product
                    sprod = adir[0]*jdir[0] + adir[1]*jdir[1] # Scalar product
                    angla = math.atan2( vprod, sprod )        # Angle from adir to jdir
                    if angla > math.pi/2: # Limit this angle
                        angla = 0
                    anglj = max(anglj, angla)
                        
                    #anglj = math.atan2( yj-y, xj-x )
                #print(anglj)
                angl = angl + anglj + math.pi/1e4 # Slightly increment the angle
                #print(angl)
                
                pdir = [ math.cos(angl), math.sin(angl) ]
                polygon.append(mine)
                #if anglj > 0:
                 #   print(anglj)
                  #  break
                
            else: # Did not find ant value
                print(('No more eligible point for polygon at sprite '+str(ppi)))
                break
        
        ppx = [ px[i] for i in polygon ]
        ppy = [ py[i] for i in polygon ]
        
        # Some checks
        if len(polygon)==1001:
            print(('Warning: Polygon generation failed for sprite '+str(ppi)))
        elif abs(max(ppx)-max(px)) > minlen or abs(min(ppx)-min(px)) > minlen \
          or abs(max(ppy)-max(py)) > minlen or abs(min(ppy)-min(py)) > minlen:
            print(('Warning: Polygon generation may have failed. Please check manually sprite '+str(ppi)))
            
        polysim = simplifyPolygon(polygon,ppx,ppy,math.pi/16) # Simplify the polygon
        
        ppxs.append(polysim[1])
        ppys.append(polysim[2])
        polyall.append(polysim[0])

    return (polyall,ppxs,ppys)
    
# Generates a XML file that contains the polygon
def generateXML(polygon,adress):
    
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

        #polyg.set('x','a')
        #polyg.set('y','b')
        x.text = strx #strx
        y.text = stry
        #polyg.text = 'bb' #stry
        
    mydata = ET.tostring(polygons, encoding="UTF-8", method="xml")
    mydata = pretty.parseString(mydata)
    mydata = mydata.toprettyxml(indent="\t",encoding="UTF-8")
    #mydata = mydata.toprettyxml(encoding="UTF-8")
    #print(type(str(mydata)))
    myfile = open(adress, "w")
    #myfile.write(str(mydata))
    myfile.write(mydata.decode("utf-8") )
    #myfile.write(mydata)
    myfile.close()

points = pointsFromPng('../naev/dat/gfx/ship/zalek/zalek_drone_scout.png',8,8,150)
polygon = polygonFromPng('../naev/dat/gfx/ship/zalek/zalek_drone_scout.png',8,8,150,3,6)

poly = polygon[0]

#Image.fromarray(picture[:,:,0])
#Image.fromarray(picture[:,:,1])
#plt.scatter(points[0][0],points[1][0]) # 16 on light drone
#plt.scatter(polygon[1][0],polygon[2][0])
#plt.scatter(points[0][27],points[1][27])
#plt.scatter(polygon[1][27],polygon[2][27])
plt.scatter(points[0][10],points[1][10])
plt.scatter(polygon[1][10],polygon[2][10])

generateXML(polygon,'zalek_drone_scout.xml')

# must refine : arx, brigand, ira, hephaestus, kestrel, lancelot, nyx, prototype, sting, vigilance
# sx and sy wrong : demon, diablo, drone, imp, mephisto, vox, zlk drone bomber, zlk drone scout