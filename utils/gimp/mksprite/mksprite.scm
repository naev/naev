; mksprite.scm
; (C) 2000 Andrew Mustun
; (C) 2008 Edgar Simo <bobbens@gmail.com>
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License version 3 as
; published by the Free Software Foundation.
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
; Creates a sprite
; February 2008 : Modified to use X and Y axis instead of just X axis
;                 inverted rotation
;                 no longer using file path as a paramater, instead using
;                 current image
;                 added undo
; April 2000    : Initial Release
(define (script-fu-naev-mksprite timg tdrawable inX inY)
  (let*
    (
      (angle (/ (* 2 3.1415926536) (* inX inY)))
      (sprite_width (car (gimp-image-width timg)))
      (sprite_height (car (gimp-image-height timg)))
      (layers 0)
      (tlayer 0)
      (new_width 0)
      (new_height 0)
      (numX 1)
      (numY 0)
      )

     (gimp-image-undo-group-start timg)

     ; Set active layer to the part we want to resize / move / rotate
     (set! layers (gimp-image-get-layers timg))
     (set! layers (cadr layers))
     (gimp-image-set-active-layer timg (aref layers 0))
     (set! tlayer (aref layers 0))

     ; Resize the image and the layer
     (set! new_width (* inX (car (gimp-image-width timg))))
     (set! new_height (* inY (car (gimp-image-height timg))))
     (gimp-image-resize timg new_width new_height 0 0)
     (gimp-layer-resize tlayer new_width new_height 0 0)

     (while (< numY inY)
            (while (< numX inX)

                   ; Copy non-rotated sprite
                   (gimp-rect-select timg 0 0 sprite_width sprite_height 2 0 0)
                   (gimp-edit-copy tdrawable)

                   ; Paste non rotated sprite
                   (gimp-edit-paste tdrawable 0)

                   ; Move Sprite to its position and anchor it
                   (gimp-layer-translate (car (gimp-image-floating-selection timg)) (* numX sprite_width) (* numY sprite_height))
                   (gimp-floating-sel-anchor (car (gimp-image-floating-selection timg)))

                   ; Rotate Sprite
                   (gimp-rect-select timg (* numX sprite_width) (* numY sprite_height) sprite_width sprite_height 2 0 0)
                   (gimp-rotate tdrawable 1 (* (* angle -1)
                                               (+ (+ (+ numX 1) (* numY inX)) -1)))
                   (gimp-floating-sel-anchor (car (gimp-image-floating-selection timg)))

                   (set! numX (+ numX 1))
                   )
            (set! numX 0)
            (set! numY (+ numY 1))
            )

     (gimp-image-undo-group-end timg)

     (gimp-displays-flush)

     ; Save image as targa
     ;(set! path inOutput)
     ;(set! tdr (car (gimp-image-active-drawable timg)))
     ;(file-tga-save 1 timg tdr path path 1)
     )
  )

(script-fu-register "script-fu-naev-mksprite"
                    _"_mksprite..."
                    "Creates a spritesheet out of a static image (facing right) for use by naev"
                    "Andrew Mustun / bobbens"
                    "Andrew Mustun / bobbens"
                    "April 2000 / March 2008"
                    "RGB*"
                    SF-IMAGE "image" 0
                    SF-DRAWABLE "drawable" 0
                    SF-VALUE "Number X:" "6"
                    SF-VALUE "Number Y:" "6" )
(script-fu-menu-register "script-fu-naev-mksprite"
                         _"<Toolbox>/Xtns/naev")
