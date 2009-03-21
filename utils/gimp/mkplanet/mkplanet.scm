; mkplanet.scm
; (C) 2005-2007 Aurore D. "Rore", aurore.d@gmail.com 
; (C) 2008 Bobbens, bobbens@gmail.com
; (C) 2009 Renderwahn
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
;
;
; Creates a planet.
; March 2009	 : Flipped bending of the shadow since mostly lit planets looked arkward.
; December 2007 : Hacked it to add customizeable glow size and changed paths to be
;                 more naevy :)
; February 2007 : Small modifications to make it work on Gimp 2.3 and hopefully 2.4
;                 Use planet-render1-1.scm for Gimp 2.2	

(define (script-fu-naev-mkplanet planetSize planetColor sunAngle sunTilt glowSize atmosDensity)

 (let*

     (
      ; 1/10th of the planet size
      (tenth (/ planetSize 10) )
      ; image size 10% wider than the planet on each side
      (imgSize (+ planetSize (* tenth 2)) )
		; getting the inverse of the glowSize so it makes more sense to the user
      (glowSizeInv (/ 1 glowSize))

      (theImage (car (gimp-image-new imgSize  imgSize RGB) ))
      (layerbase (car (gimp-layer-new theImage imgSize imgSize 0 "planet base" 100 NORMAL) ) )
      (layeratmosph (car (gimp-layer-new theImage imgSize imgSize 0 "planet atmosphere" 100 NORMAL) ) )
      (layershadowemp (car (gimp-layer-new theImage imgSize imgSize 0 "planet shadow temp" 100 NORMAL) ) )
      (layershadow (car (gimp-layer-new theImage imgSize imgSize 0 "planet shadow" 100 NORMAL) ) )
      (layerglow (car (gimp-layer-new theImage imgSize imgSize 0 "planet glow" 100 NORMAL) ) )
      (angleRad (/ (* sunAngle *pi*) 180))
      (transX (* (sin angleRad) -1))
      (transY (cos angleRad))
     )

   (gimp-context-push)
   (gimp-image-undo-disable theImage)

   (gimp-layer-add-alpha layerbase )
   (gimp-layer-add-alpha layeratmosph )
   (gimp-layer-add-alpha layershadowemp )
   (gimp-layer-add-alpha layershadow )
   (gimp-layer-add-alpha layerglow )

   (gimp-image-add-layer theImage layerglow 0)
   (gimp-image-add-layer theImage layerbase 0)
   (gimp-image-add-layer theImage layeratmosph 0)
   (gimp-image-add-layer theImage layershadowemp 0)
   (gimp-image-add-layer theImage layershadow 0)

   (gimp-selection-all theImage)
   (gimp-edit-clear layerbase)
   (gimp-edit-clear layeratmosph)
   (gimp-edit-clear layershadowemp)
   (gimp-edit-clear layershadow)
   (gimp-edit-clear layerglow)
   (gimp-selection-none theImage)


	;it is filled with black, and the inverse of the shadow is drawn with white on it, then white is changed into alpha
   (gimp-selection-all theImage);;;
   (gimp-context-set-background '(0 0 0) );;;
   (gimp-edit-bucket-fill layershadow 1 0 100 0 FALSE 0 0 );;;
   (gimp-selection-none theImage);;;


   (gimp-ellipse-select theImage tenth tenth planetSize planetSize 2 1 0 0 )
   (gimp-context-set-foreground planetColor)
   ; fill selection with the planet color
   (gimp-edit-bucket-fill layerbase 0 0 100 0 FALSE 0 0 )
   (gimp-edit-bucket-fill layeratmosph 0 0 100 0 FALSE 0 0 )
   
    
   ; shrink and blur for the shadow
   (gimp-selection-shrink theImage (* 1.5 tenth ));;;
   
   (gimp-selection-feather theImage (* 1.5 tenth) )
   (gimp-context-set-background '(255 255 255) )
   (gimp-edit-bucket-fill layershadowemp 1 0 100 0 FALSE 0 0 );;
   
   
   ; add the light around the planet for the atmosphere
   (gimp-selection-layer-alpha layeratmosph)
   ;(gimp-selection-shrink theImage tenth)
   (gimp-selection-shrink theImage (* tenth atmosDensity))
   (gimp-selection-feather theImage (* atmosDensity tenth)) 
   (gimp-layer-set-lock-alpha layeratmosph 1)
   (gimp-selection-invert theImage)
   (gimp-context-set-background '(255 255 255) )
   (gimp-edit-bucket-fill layeratmosph 1 5 90 0 FALSE 0 0 )
   (gimp-selection-invert theImage)
   (gimp-context-set-foreground '(0 0 0) )
   (gimp-edit-bucket-fill layeratmosph 0 0 100 0 FALSE 0 0 )
   (gimp-selection-layer-alpha layeratmosph)
   (gimp-selection-shrink theImage (/ tenth 3))
   (gimp-selection-feather theImage tenth)
   (gimp-selection-invert theImage)
   (gimp-edit-bucket-fill layeratmosph 1 0 85 0 FALSE 0 0 )
   (gimp-layer-set-mode layeratmosph 4) 
   
   ;move,resize the shadow layer
   (gimp-layer-scale layershadowemp (* (+ 1.5 (/ sunTilt 10)) imgSize)  (* (+ 1.5 (/ sunTilt 10)) imgSize) 1 );;;
   (gimp-layer-translate layershadowemp (* (* transX tenth) (+ 1 sunTilt) ) (* (* transY tenth) (+ 1 sunTilt) ) );;;
   
	;;make the actual shadow layer from the temp layer and the shadow layer
	(gimp-selection-all theImage);;;
   	(gimp-edit-copy layershadowemp);;;
	(gimp-floating-sel-anchor (car (gimp-edit-paste layershadow FALSE)));;;
	;(gimp-invert layershadow)
	(plug-in-colortoalpha 1 theImage layershadow '(255 255 255));;;
	(gimp-drawable-transform-rotate-simple layershadow 1 TRUE 0 0 TRUE);;;
	(gimp-drawable-set-visible layershadowemp FALSE);;;
   
   ; and now the glow...
   (gimp-selection-layer-alpha layerbase)
   (gimp-selection-grow theImage (/ tenth glowSizeInv))
   (gimp-selection-feather theImage tenth )
   (gimp-context-set-background planetColor )
   (gimp-edit-bucket-fill layerglow  1 0 100 0 FALSE 0 0 )
   (gimp-edit-bucket-fill layerglow  1 7 100 0 FALSE 0 0 )
   (gimp-edit-bucket-fill layerglow  1 7 100 0 FALSE 0 0 )
   ; mask a part of the glow
   (let ((glowmask (car (gimp-layer-create-mask layerglow 0))))
     (gimp-layer-add-mask layerglow glowmask)
     (gimp-selection-layer-alpha layershadow)
     (gimp-edit-bucket-fill glowmask  0 0 100 0 FALSE 0 0 )
   )
   (gimp-selection-all theImage) ;; stuff to remove ugly thin white border
   (gimp-fuzzy-select layerbase (/ imgSize 2) (/ imgSize 2) 15 1 1 0 0 0) ;;  
   (gimp-edit-cut layershadow)
   (gimp-layer-resize-to-image-size layershadow)
   
   
   (gimp-image-undo-enable theImage)
   (gimp-display-new theImage)
   (gimp-context-pop)
   )
 )

(script-fu-register "script-fu-naev-mkplanet"
                   _"_mkplanet..."
                   "Creates a planet.(Color, size and sun orientation can be set)"
                   "Aurore D. (Rore) / wolorf / bobbens"
                   "aurore.d@gmail.com / wolorf@gmail.com / bobbens@gmail.com"
                   "October 2005 / september 2006 / december 2007"
                   ""
                   SF-ADJUSTMENT _"Planet size (pixels)" '(400 40 2000 1 10 0 1)
                   SF-COLOR _"Planet color" '(10 70 100)
                   SF-ADJUSTMENT _"Sun orientation (degrees) " '(45 0 360 1 10 1 0) 
		             SF-ADJUSTMENT _"Sun Tilt " '(3.6 0 10 1 10 1 0)
                   SF-ADJUSTMENT _"Planet Glow" '(.25 .1 .6667 1 2 1 0)
                   SF-ADJUSTMENT _"Atmosphere Density" '(2 1 6 1 1 0 0) )
(script-fu-menu-register "script-fu-naev-mkplanet"
			                _"<Toolbox>/Xtns/naev")
