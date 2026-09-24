;;; begin-user-library
;; This block describes the library of user functions.
;; So the editor knows that your device implements it.
;
; (defjs bclicked (button_id)) ;-> Bool
; (defjs vibro (_0)) ;-> Bool
; (defjs tof_distance ()) ;-> Int
; (defjs pixel_clear ()) ;-> Bool
; (defjs pixel_set (_0 _1 _2 _3)) ;-> Bool
; (defjs pixel_show ()) ;-> Bool
;
;;; end-user-library

(define led ())
(define red ())
(define run ())
(define led_fill ())
(define green ())
(define rise ())
(define blue ())
(define delta ())
(define distance ())
; Re-maps a number from one range to another.
(defun map
 (x in_min in_max out_min out_max)
 (setq run
  (- in_max in_min))
 (setq rise
  (- out_max out_min))
 (setq delta
  (- x in_min))
 (/
  (* delta rise)
  (+ run out_min)))

(setq led 0)
(setq led_fill ())

(setq red 10)
(setq green 10)
(setq blue 10)

(task 0 80 '
 (progn
  (if
   (is_event 'red)
   (progn
    (setq red
     (pop_event 'red))))
  (if
   (is_event 'green)
   (progn
    (setq green
     (pop_event 'green))))
  (if
   (is_event 'blue)
   (progn
    (setq blue
     (pop_event 'blue))))
  (if
   (bclicked 0)
   (progn
    (setq led_fill
     (not
      (bool led_fill)))
    (vibro 2)))
  (setq distance
   (tof_distance))
  (setq led
   (map distance 40 360 0 10))
  (pixel_clear)
  (if
   (bool led_fill)
   (progn
    (while
     (< #itr
      (+ led 1))
     (pixel_set #itr red green blue)))
   (progn
    (pixel_set led red green blue)))
  (pixel_show)
  (if
   (is_event 'capture)
   (progn
    (if
     (eql
      (pop_event 'capture) 1)
     (progn
      (push_event 'distance distance)))))))
