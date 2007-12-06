; naev-planet.creator.scm
; Creates a planet.
; Aurore D. (Rore) 2005
;
; Script edited for purposes of NAEV (Not Another Escape Velocity) https://trac.bobbens.dyndns.org/naev/wiki
; Copyright (C) 2006, by Joel Hans
; wolorf@gmail.com
;
; Many thanks to Aurore for his work on the original version of this script.
;
; Added features:
; 1. increased range for the sun tilt. Now all of the planet can be lit if desired.
; 2. Allowed the user to change how much glow there is on the planet.
;
; The script will create its own menu in the toolbox.
;
; Distributed under GNU/GPL license: http://www.gnu.org/copyleft/gpl.html

(define (naev-planet-creator planetSize planetColor sunAngle sunTilt glowSize)

  (let*

	 (
	  ; 1/10th of the planet size
	  (tenth (/ planetSize 10) )

	  ; image size 10% bigger than the planet
	  (imgSize (+ planetSize (* tenth 2)) )

	  ; getting the inverse of the glowSize so it makes more sense to the user
	  (glowSizeInv (/ 1 glowSize))

	  (theImage (car (gimp-image-new imgSize  imgSize RGB) ))
	  (layerbase (car (gimp-layer-new theImage imgSize imgSize 0 "planet base" 100 NORMAL) ) )
	  (layeratmosph (car (gimp-layer-new theImage imgSize imgSize 0 "planet atmosphere" 100 NORMAL) ) )
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
	 (gimp-layer-add-alpha layershadow )
	 (gimp-layer-add-alpha layerglow )

	 (gimp-image-add-layer theImage layerglow 0)
	 (gimp-image-add-layer theImage layerbase 0)
	 (gimp-image-add-layer theImage layeratmosph 0)
	 (gimp-image-add-layer theImage layershadow 0)

	 (gimp-selection-all theImage)
	 (gimp-edit-clear layerbase)
	 (gimp-edit-clear layeratmosph)
	 (gimp-edit-clear layershadow)
	 (gimp-edit-clear layerglow)
	 (gimp-selection-none theImage)


	 (gimp-ellipse-select theImage tenth tenth planetSize planetSize 2 0 0 0 )
	 (gimp-context-set-foreground planetColor)

	 ; fill selection with the planet color
	 (gimp-edit-bucket-fill layerbase 0 0 100 0 FALSE 0 0 )
	 (gimp-edit-bucket-fill layeratmosph 0 0 100 0 FALSE 0 0 )

	 ; shrink and blur for the shadow
	 (gimp-selection-feather theImage (* 1.5 tenth) )
	 (gimp-context-set-background '(0 0 0) )
	 (gimp-edit-bucket-fill layershadow 1 0 100 0 FALSE 0 0 );;

	 ; add the light around the planet for the atmosphere
	 (gimp-selection-layer-alpha layeratmosph)
	 (gimp-selection-shrink theImage tenth)
	 (gimp-selection-feather theImage (* 2 tenth))
	 (gimp-layer-set-preserve-trans layeratmosph 1)
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

	 ; move,resize the shadow layer
	 (gimp-layer-scale layershadow (* (+ 1.5 (/ sunTilt 10)) imgSize)  (* (+ 1.5 (/ sunTilt 10)) imgSize) 1 )
	 (gimp-layer-translate layershadow (* (* transX tenth) (+ 3 sunTilt) ) (* (* transY tenth) (+ 3 sunTilt) ) )

	 ; and now the glow...
	 (gimp-selection-layer-alpha layerbase)
	 (gimp-selection-grow theImage (/ tenth glowSizeInv))
	 (gimp-selection-feather theImage tenth)
	 (gimp-context-set-background planetColor )
	 (gimp-edit-bucket-fill layerglow  1 0 100 0 FALSE 0 0 )
	 (gimp-edit-bucket-fill layerglow  1 7 100 0 FALSE 0 0 )
	 (gimp-edit-bucket-fill layerglow  1 7 100 0 FALSE 0 0 )

	 ; mask a part of the glow
	 (set! glowmask (car (gimp-layer-create-mask layerglow 0)))
	 (gimp-image-add-layer-mask theImage layerglow glowmask)
	 (gimp-selection-layer-alpha layershadow)
	 (gimp-edit-bucket-fill glowmask  0 0 100 0 FALSE 0 0 )
	 (gimp-selection-all theImage)
	 (gimp-fuzzy-select layerbase (/ imgSize 2) (/ imgSize 2) 15 1 1 0 0 0)
	 (gimp-edit-cut layershadow)
	 (gimp-layer-resize-to-image-size layershadow)

	 (gimp-image-clean-all theImage)
	 (gimp-image-undo-enable theImage)
	 (gimp-display-new theImage)
	 (gimp-context-pop)
	 )
  )

(script-fu-register "naev-planet-creator"
						  _"_NAEV Planet Creator"
						  "Creates a planet. (For use in Not Another Escape Velocity)"
						  "Aurore D. (Rore) / wolorf"
						  "aurore.d@gmail.com / wolorf@gmail.com"
						  "October 2005 / September 2006"
						  ""
						  SF-ADJUSTMENT "Planet Size (pixels)" '(80 40 2000 1 10 0 1)
						  SF-COLOR "Planet Color" '(10 70 100)
						  SF-ADJUSTMENT _"Sun Orientation (degrees) " '(0 0 360 1 10 1 0) 
						  SF-ADJUSTMENT _"Sun Tilt " '(1 0 25 1 10 1 0)
						  SF-ADJUSTMENT _"Glow Size" '(.25 .1 .6667 1 2 1 0) 
						  )
(script-fu-menu-register "script-fu-naev-planet-render2"
								 _"<Toolbox>/NAEV/")
