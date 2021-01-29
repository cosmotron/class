;; Ryan Lewis - 0147687
;; Worked with Justin Bennett
;; Assignment #5 - 8 Puzzler w/ Graphics (our picture for the contest is out board)
;;
;; Use (play) to start the game.  It will writeln "Shuffle" when shuffling and then
;;   writeln "Solving" when solving (this is all in the command line, so keep and eye on it)

;; board
;;  the object (message passing) implementation of the sliding puzzle board
;;  internally uses the built-in VECTOR structure (resizeable arrays)
;;
(define board
  (lambda () 
    ;; use vector instead of make-initialized-vector because drscheme doesn't have the later
    (let ((board (vector 0 1 2 3 4 5 6 7 8)) 
          (blank 0)
          (step 0)
          (center 4))
      (letrec ((this (lambda message 
                       (case (car message) 
                         ((size) 
                          ;; returns the total number of cells on the board
                          9) 
                         ((get-item) 
                          ;; returns the tile on position (x,y) on the board
                          (board-ref board 3 (cadr message) (caddr message)))
                         ((get-position)
                          ;; returns the flat position of the tile 
                          (let seek ((pos 0))
                            (if (= (cadr message) (vector-ref board pos))
                                pos
                                (seek (+ pos 1)))))

                         ;;;;;;;;;;;;;;;;;;;;;;;;
                         ;; show the board and clear each time so everything isn't overlapped
                         ((show) 
                          (graphics-clear w)
                          ;; outputs the board 
                          (display-board (vector->list board) 3 3)
                          (draw-board w (vector->list board) 0)
                          (sleep 1000))

                         ;;;;;;;;;;;;;;;;;;;;;;;;
                         ;; will move the blank to the middle
                         ((blank-to-center)
                          (begin
                            (this (cadr (assq blank oklahoma)))
                            (this (cadr (assq blank oklahoma)))))

                         ((stay) (this 'show))
                         ((move-down) 
                          ;; moves the blank tile down one position
                          (if (not (last-row? blank 3)) 
                              (let ((i (get-row blank 3)) (j (get-column blank 3))) 
                                (let ((item (board-ref board 3 (+ i 1) j))) 
                                  (begin 
                                    (vector-set! board blank item) 
                                    (vector-set! board (flat-index (+ i 1) j 3) 0) 
                                    (set! blank (flat-index (+ i 1) j 3))
                                    (this 'show))))))
                         ((move-up) 
                          ;; moves the blank tile up one position
                          (if (not (first-row? blank 3))
                              (let ((i (get-row blank 3)) (j (get-column blank 3)))
                                (let ((item (board-ref board 3 (- i 1) j)))
                                      (begin
                                        (vector-set! board blank item)
                                        (vector-set! board (flat-index (- i 1) j 3) 0)
                                        (set! blank (flat-index (- i 1) j 3))
                                        (this 'show))))))
                         ((move-left)
                         ;; moves the blank tile to the left one position
                          (if (not (first-column? blank 3))
                              (let ((i (get-row blank 3)) (j (get-column blank 3)))
                                (let ((item (board-ref board 3 i (- j 1))))
                                  (begin
                                    (vector-set! board blank item)
                                    (vector-set! board (flat-index i (- j 1) 3) 0)
                                    (set! blank (flat-index i (- j 1) 3))
                                    (this 'show))))))
                         ;; moves the blank tile to the right one position
                         ((move-right)                     
                          (if (not (last-column? blank 3))
                              (let ((i (get-row blank 3)) (j (get-column blank 3)))
                                (let ((item (board-ref board 3 i (+ j 1))))
                                  (begin
                                    (vector-set! board blank item)
                                    (vector-set! board (flat-index i (+ j 1) 3) 0)
                                    (set! blank (flat-index i (+ j 1) 3))
                                    (this 'show))))))
                         ;; shuffle by randomly picking a valid direction to go
                         ((shuffle)
                          (writeln '(shuffling))
                          ;; randomly shuffles the board a number of times
                          (let shuffle ((times (cadr message)))
                            (if (= 0 times)
                                (this 'show)
                                (let ((step (select-any-from-list (list 'move-up 'move-down 'move-left 'move-right))))
                                  (this step)
                                  (shuffle (- times 1))))))

                         ;; Use the around-the-world data structure once
                         ((circumnavigate-the-globe)
                          (if (not (= blank 4)) (this (cadr (assq blank around-the-world)))))
                         ;; Use the nor-easter data structure once
                         ((nor-east-rotate)
                          (this (cadr (assq blank nor-easter))))
                         ;; Use the west-side data structure once
                         ((west-side-rotate)
                          (this (cadr (assq blank west-side))))
                         ;; Use the bering-strait data structure once
                         ((bs-rotate) ;;bs for bering-strait...
                          (this (cadr (assq blank bering-strait))))
                         ;; Use the potsdam data structure once
                         ((potsdam-rotate) ;;good 'ol 44 degrees north latitude
                          (this (cadr (assq blank potsdam))))
                         ;; Use the tex-mex data structure once
                         ((tex-mex-rotate) ;;kind of like la cucaracha
                          (this (cadr (assq blank tex-mex))))
                         ((solved?) 
                          ; checks if the puzzle is solved
                          (equal? board (vector 0 1 2 3 4 5 6 7 8)))

                         ;;;;;;;;;;;;;;;;;;;;;;;;
                         ;; will check to see if 8 is in position
                         ;; if it isn't, put the blank in the center, move the blank out of the center
                         ;;   and then use circumnavigate-the-globe to rotate 8 into it's position (lower-right)
                         ((place-8)
                          ;; Feature Implemented - December 4, 2007 19:23:34.67
                          (let ((pos8 (this 'get-position 8)))
                            (if (not (= 8 pos8))
                                (begin
                                  (this 'blank-to-center)
                                  (let ((pos8 (this 'get-position 8)))                                        
                                    (case pos8
                                      ((5) (this 'move-left))
                                      (else (this 'move-right))))
                                  (let loop ((pos8 (this 'get-position 8)))
                                    (if (not (= 8 pos8))
                                        (begin (this 'circumnavigate-the-globe)
                                               (loop (this 'get-position 8)))))))))

                         ;;;;;;;;;;;;;;;;;;;;;;;;
                         ;; check to see if 7 is in position
                         ;; if it isn't, blank-to-center and if 7 is in the  third column, move it out
                         ;;   so ultimately, nor-easter can rotate it into position (lover-second column)
                         ;;
                         ((place-7)
                          (let ((pos7 (this 'get-position 7)))
                            (if (not (= 7 pos7))
                                (begin
                                  (this 'blank-to-center)
                                  (let ((pos7 (this 'get-position 7)))
                                    (case pos7
                                      ((2 5) 
                                       (let loop ((pos7 (this 'get-position 7)))
                                         (if (not (= 1 pos7))
                                             (begin (this 'nor-east-rotate)
                                                    (loop (this 'get-position 7)))))))                                       
                                    (let loop ((pos7 (this 'get-position 7)))
                                      (if (not (= 7 pos7))
                                          (begin (this 'west-side-rotate)
                                                 (loop (this 'get-position 7))))))))))

                         ;;;;;;;;;;;;;;;;;;;;;;;;
                         ;; check to see if 6 is in position
                         ;; if it isn't, blank-to-center, put 6 in the top-left, put 7 in left-second-row,
                         ;;   use west-side to rotate them both into position
                         ;;
                         ((place-6)
                          (let ((pos6 (this 'get-position 6)))
                            (if (not (= 6 pos6))
                                (begin
                                  (this 'blank-to-center)
                                  (if (not (= 6 (this 'get-position 6)))                                      
                                      (begin
                                        (let loop ((pos6 (this 'get-position 6)))
                                          (if (not (= 0 pos6))
                                              (begin (this 'potsdam-rotate)
                                                     (loop (this 'get-position 6)))))
                                        (let loop2 ((pos7 (this 'get-position 7)))
                                          (if (not (= 3 pos7))
                                              (begin (this 'tex-mex-rotate)
                                                     (loop2 (this 'get-position 7)))))
                                        (let loop3 ((pos7 (this 'get-position 7)))
                                          (if (not (= 7 pos7))
                                              (begin (this 'west-side-rotate)
                                                     (loop3 (this 'get-position 7)))))))))))
                         ;;;;;;;;;;;;;;;;;;;;;;;;
                         ;; check to see if 2 is in position
                         ;; if it isn't, blank-to-center, put 2 in the top-right by using potsdam-rotate
                         ((place-2)
                          (let ((pos2 (this 'get-position 2)))
                            (if (not (= 2 pos2))
                                (begin
                                  (this 'blank-to-center)
                                  (if (not (= 2 (this 'get-position 2)))
                                      (begin
                                        (let loop ((pos2 (this 'get-position 2)))
                                          (if (not (= 2 pos2))
                                              (begin (this 'potsdam-rotate)
                                                     (loop (this 'get-position 2)))))))))))
                         ;;;;;;;;;;;;;;;;;;;;;;;;
                         ;; check to see if 5 is in position
                         ;; if it isn't, blank-to-center, put 5 in middle-second-row and then use potsdamn-rotate
                         ;;   to put 2 and 5 into place
                         ((place-5)
                          (let ((pos5 (this 'get-position 5)))
                            (if (not (= 5 pos5))
                                (begin
                                  (this 'blank-to-center)
                                  (if (not (= 5 (this 'get-position 5)))
                                      (begin
                                        (let loop ((pos5 (this 'get-position 5)))
                                          (if (not (= 3 pos5))
                                              (begin (this 'bs-rotate)
                                                     (loop (this 'get-position 5)))))
                                        (let loop2 ((pos2 (this 'get-position 2)))
                                          (if (not (= 4 pos2))
                                              (begin (this 'nor-east-rotate)
                                                     (loop2 (this 'get-position 2)))))
                                        (let loop3 ((pos2 (this 'get-position 2)))
                                          (if (not (= 2 pos2))
                                              (begin (this 'potsdam-rotate)
                                                     (loop3 (this 'get-position 2)))))))))))

                         ;;;;;;;;;;;;;;;;;;;;;;;;
                         ;; will call the above functions then once 8, 7, 6, 2, 5 are in place, it just needs to
                         ;;   bs-rotate 1, 3, 4 into place.
                         ((solve)
                          (writeln '(now solving))
                          (if (not (this 'solved?))
                              (begin
                                (this 'place-8)
                                (this 'place-7)
                                (this 'place-6)
                                (this 'place-2)
                                (this 'place-5)
                                (if (not (this 'solved?))
                                    (let loop ((pos1 (this 'get-position 1)))
                                      (if (= 1 pos1)
                                          (begin (this 'bs-rotate)
                                                 (loop (this 'get-position 1))))))                               
                                (let loop ((pos1 (this 'get-position 1)))                              
                                  (if (not (= 1 pos1))
                                      (begin (this 'bs-rotate)
                                             (loop (this 'get-position 1))))))))
                         ((error) 
                          ;; reports an error message
                          (writeln (cadr message)))
                         (else 
                          ;; message to the board object is not recognized
                          (writeln "bad message to board object"))))))
        this))))

(define t (board))

;;;;;;;;;;;;;;;;;;;;;;;;
;; Call this to shuffle then solve
(define play
  (lambda ()
    (t 'shuffle 70)
    (t 'solve)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; rotation data structures
;;

;;;;;;;;;;;;;;;;;;;;;;;;
;; rotates the digits clockwise around the center piece
;;
(define around-the-world
  '((0 move-down)  (1 move-left)  (2 move-left)
    (3 move-down)  (4 stay)       (5 move-up)
    (6 move-right) (7 move-right) (8 move-up)))

;;;;;;;;;;;;;;;;;;;;;;;;
;; rotates the first and second column clockwise
;;
(define west-side
  '((0 move-down)  (1 move-left)  (2 stay)
    (3 move-down)  (4 move-up)    (5 stay)
    (6 move-right) (7 move-up)    (8 stay)))

;;;;;;;;;;;;;;;;;;;;;;;;
;; rotates the intersection of the first and second row
;;   with the second and third column clockwise
;;
(define nor-easter
  '((0 stay) (1 move-down)  (2 move-left)
    (3 stay) (4 move-right) (5 move-up)
    (6 stay) (7 stay)       (8 stay)))

;;;;;;;;;;;;;;;;;;;;;;;;
;; rotates the intersection of the first and second row
;;   with the first and second column clockwise
;;
(define bering-strait ;;because its in the north-west :)
  '((0 move-down)  (1 move-left) (2 stay)
    (3 move-right) (4 move-up)   (5 stay)
    (6 stay)       (7 stay)      (8 stay)))

;;;;;;;;;;;;;;;;;;;;;;;;
;; moves the blank to the center
;;
(define oklahoma ;;because its the heart of the world?
  '((0 move-down)  (1 move-down) (2 move-down)
    (3 move-right) (4 stay)      (5 move-left)
    (6 move-up)    (7 move-up)   (8 move-up)))

;;;;;;;;;;;;;;;;;;;;;;;;
;; rotates the first and second row clockwise
;;
(define potsdam ;;you know... the north country
  '((0 move-down)  (1 move-left)  (2 move-left)
    (3 move-right) (4 move-right) (5 move-up)
    (6 stay)       (7 stay)       (8 stay)))

;;;;;;;;;;;;;;;;;;;;;;;;
;; rotates the intersection of the second and third row
;;  with the first and second column clockwise
;;
(define tex-mex ;;southwestern cuisine
  '((0 stay)       (1 stay)      (2 stay)
    (3 move-down)  (4 move-left) (5 stay)
    (6 move-right) (7 move-up)   (8 stay)))
;;
;; end rotation data structures
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; board-ref
;;  given row and column on a board of size x size, 
;;  returns the tile item at that position
;; uses the built-in vector-ref function for accessing vector elements
;;
(define board-ref
  (lambda (board size row column) 
    (vector-ref board (flat-index row column size))))

;; flat-index
;;  given row and column on a board of size x size, 
;;  returns the flat vector index of that position
;;
(define flat-index
  (lambda (row column size)
    (+ (* row size) column)))

;; get-row 
;;  given a flat vector index on the board of size x size, 
;;  returns the associated row position
;;
(define get-row
  (lambda (flat-index size)
    (quotient flat-index size)))

;; get-column
;;  given a flat vector index on the board of size x size, 
;;  returns the associated column position
;;
(define get-column
  (lambda (flat-index size)
    (remainder flat-index size)))

;; last-row?
;;  given a flat vector index on the board of size x size, 
;;  decides if that position is on the last row
;;
(define last-row?
  (lambda (flat-index size)
    (= (+ (get-row flat-index size) 1) size)))

;; last-column?
;;  given a flat vector index on the board of size x size, 
;;  decides if that position is on the last column
;;
(define last-column?
  (lambda (flat-index size)
    (= (+ (get-column flat-index size) 1) size)))

;; first-row?
;;  given a flat vector index on the board of size x size, 
;;  decides if that position is on the first row
;;
(define first-row?
  (lambda (flat-index size)
    (= 0 (get-row flat-index size))))

;; first-column?
;;  given a flat vector index on the board of size x size, 
;;  decides if that position is on the first column
;;
(define first-column?
  (lambda (flat-index size)
    (= 0 (get-column flat-index size))))

;; display-board
;;  displays the board of size x size to the text screen
;;  as a list of size lists each of length size
;;
(define display-board
  (lambda (board size size2)
    (cond ((null? board) '())
          ((= size2 1) (writeln board) (newline))
          (else (writeln (get-next-so-many board size))
                (display-board (trim-so-many board size) size (- size2 1))))))

;; get-next-so-many
;;  helper function for display-board, gets next size elements from board
;;
(define get-next-so-many
  (lambda (board size)
    (cond ((null? board) '())
          ((= 1 size) (cons (car board) '()))
          (else (cons (car board) (get-next-so-many (cdr board) (- size 1)))))))

;; trim-so-many
;;  helper function for display-board, trims size elements from front of board
(define trim-so-many
  (lambda (board n)
    (cond ((null? board) '())
          ((= n 1) (cdr board))
          (else (trim-so-many (cdr board) (- n 1))))))

;; writeln
;;  writes an object to the text screen followed by a newline
;;
(define writeln
  (lambda (object)
    (write object)
    (newline)))

;; sleep
;;  wastes a few clock cycles to allow slow human responses to catch up
;;
(define sleep
  (lambda (ticks) 
    (let ((current-time (process-time-clock))) 
      (let ((end-time (+ current-time ticks))) 
        (let wait () 
          (if (> end-time (process-time-clock)) 
              (wait)))))))

;; square
;;  a numeric function that squares its input
;;
(define square
  (lambda (n)
    (* n n)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; miscellaneous helper functions
;;
;;

;;list-ref: returns the list item at index
;; 
(define list-ref
  (lambda (ls index)
    (cond ((null? ls) '())
          ((= index 0) (car ls))
          (else (list-ref (cdr ls) (- index 1))))))

;;select-any-from-list: selects a random item from the list
;;
(define select-any-from-list
  (lambda (ls)
    (list-ref ls (random (length ls)))))
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; graphics functions
;;

;; creates a window
;;
(define make-window
  (lambda (x-size y-size foreground-color background-color)
    ;;(let ((window (make-graphics-device 'x)))						
    ;;(let ((window (make-graphics-device 'win32 x-size y-size)))
    (let ((window (make-graphics-device 'x x-size y-size)))
      (begin
        (graphics-set-coordinate-limits window 0 0 x-size y-size)
        (set-foreground-color window foreground-color)
        (set-background-color window background-color)
        (graphics-clear window)
        window))))

;; sets the foreground color of a specified window
;;
(define set-foreground-color
  (lambda (window color)
    (graphics-operation window 'set-foreground-color color)))

;; sets the background color of a specified window
;;
(define set-background-color
  (lambda (window color)
    (graphics-operation window 'set-background-color color)))

;; kills a specified window
;;
(define kill-window
  (lambda (window)
    (graphics-close window)))

;; draws a line from (x1,y1) to (x2,y2)
;;
(define draw-line
  (lambda (window x1 y1 x2 y2)
    (graphics-operation window 'draw-line x1 y1 x2 y2)))

;; draw the vertical and horizontal slide piece separators
;;
(define draw-board-lines
  (lambda (window)
    (begin
      (draw-line window 200 0 200 600)
      (draw-line window 400 0 400 600)
      (draw-line window 0 200 600 200)
      (draw-line window 0 400 600 400))))

;;;;;;;;;;;;;;;;;;;;;;;;
;; draw individual sections of a 7-section digit
;;
;;     __(1)__
;; (2) |     | (4)
;;     |_(3)_|
;; (5) |     | (7)
;;     |_(6)_|
;; 
(define section1
  (lambda (x y window)
    (draw-line window (+ x 50) (+ y 50) (- x 50) (+ y 50))))

(define section2
  (lambda (x y window)
    (draw-line window (- x 50) (+ y 50) (- x 50) y)))

(define section3
  (lambda (x y window)
    (draw-line window (+ x 50) (+ y 50) (+ x 50) y)))

(define section4
  (lambda (x y window)
    (draw-line window (+ x 50) y (- x 50) y)))

(define section5
  (lambda (x y window)
    (draw-line window (- x 50) (- y 50) (- x 50) y)))

(define section6
  (lambda (x y window)
    (draw-line window (+ x 50) (- y 50) (+ x 50) y)))

(define section7
  (lambda (x y window)
    (draw-line window (+ x 50) (- y 50) (- x 50) (- y 50))))
;;
;; end number sections
;;;;;;;;;;;;;;;;;;;;;;;;
;; draw* functions call the above functions to construct a digit
;;
(define draw1
  (lambda (x y window)
    (begin (section3 x y window) (section6 x y window))))

(define draw2
  (lambda (x y window)
    (begin (section1 x y window) (section3 x y window)
           (section4 x y window) (section5 x y window)
           (section7 x y window))))

(define draw3
  (lambda (x y window)
    (begin (section1 x y window) (section3 x y window)
           (section4 x y window) (section6 x y window)
           (section7 x y window))))

(define draw4
  (lambda (x y window)
    (begin (section2 x y window) (section3 x y window)
           (section4 x y window) (section6 x y window))))

(define draw5
  (lambda (x y window)
    (begin (section1 x y window) (section2 x y window)
           (section4 x y window) (section6 x y window)
           (section7 x y window))))

(define draw6
  (lambda (x y window)
    (begin (section1 x y window) (section2 x y window)
           (section4 x y window) (section5 x y window)
           (section6 x y window) (section7 x y window))))

(define draw7
  (lambda (x y window)
    (begin (section1 x y window) (section3 x y window)
           (section6 x y window))))

(define draw8
  (lambda (x y window)
    (begin (section1 x y window) (section2 x y window)
           (section3 x y window) (section4 x y window)
           (section5 x y window) (section6 x y window)
           (section7 x y window))))
;;
;; end digit constructors
;;;;;;;;;;;;;;;;;;;;;;;;
;; map consists of the centers of each slider box on a 600x600 game board
;;
(define *map*
  '((0 (100 500)) (1 (300 500)) (2 (500 500))
    (3 (100 300)) (4 (300 300)) (5 (500 300))
    (6 (100 100)) (7 (300 100)) (8 (500 100))))

;;;;;;;;;;;;;;;;;;;;;;;;
;; uses the order of the board vector and *map* to determine the digit to draw
;; *map* is called in order (since n increases: 0, 1, 2, ..., 8)  and as it
;;   iterates through the board vector, the correct digit is drawn at the location
;;   specified by *map*.
;;
(define draw-board
  (lambda (window board n)
    (draw-board-lines window)
    (if (not (= n 9))        
        (begin          
          (case (car board)
            ((1) (draw1 (caadr (assq n *map*)) (cadadr (assq n *map*)) window))
            ((2) (draw2 (caadr (assq n *map*)) (cadadr (assq n *map*)) window))
            ((3) (draw3 (caadr (assq n *map*)) (cadadr (assq n *map*)) window))
            ((4) (draw4 (caadr (assq n *map*)) (cadadr (assq n *map*)) window))
            ((5) (draw5 (caadr (assq n *map*)) (cadadr (assq n *map*)) window))
            ((6) (draw6 (caadr (assq n *map*)) (cadadr (assq n *map*)) window))
            ((7) (draw7 (caadr (assq n *map*)) (cadadr (assq n *map*)) window))
            ((8) (draw8 (caadr (assq n *map*)) (cadadr (assq n *map*)) window))
            (else #t))
          (draw-board window (cdr board) (+ n 1))))))

;;;;;;;;;;;;;;;;;;;;;;;;
;; create the window
;;
(define w (make-window 600 600 "white" "black"))
