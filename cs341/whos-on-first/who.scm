; Ryan Lewis - 0147687
; Worked with Justin Bennett
; Assignment #4

; whos: start the program loop.
;
(define whos
  (lambda ()
    (whos-on-first-loop '())))

; whos-on-first-loop: read user input and determine the appropriate response.
;
(define whos-on-first-loop 
  (lambda (old-context)
    (let ((costellos-line (read)))
      (let ((new-context (get-context costellos-line old-context)))
        (let ((strong-reply (try-strong-cues costellos-line)))
          (let ((weak-reply (try-weak-cues costellos-line new-context))) 
               (cond ((not (null? strong-reply))
                      (writeln strong-reply)
                      (whos-on-first-loop (get-context strong-reply new-context)))
                     ((not (null? weak-reply))
                      (writeln weak-reply)
		      (whos-on-first-loop (get-context weak-reply new-context)))
                     ((wants-to-end? costellos-line)
                      (wrap-it-up))
                     (else 
                      (writeln (hedge))
	                      (whos-on-first-loop new-context)))))))))

; get-context: determine if anything in *context-words* is in the sentence thus setting
;              the context for the user input.
;
(define get-context
  (lambda (sentence context)
    (define traverse
      (lambda (ls)
        (cond ((null? ls) context) 
              ((any-good-fragments? (cue-part (car ls)) sentence) (cdar ls))
              (else (traverse (cdr ls))))))
    (traverse *context-words*)))

; try-strong-cues: check the *strong-cues* structure for an appropriate response.
;
(define try-strong-cues
  (lambda (sentence)
    (define traverse
      (lambda (ls)
        (cond ((null? ls) '())
              ((any-good-fragments? (cue-part (car ls)) sentence)
               (select-any-from-list (response-part (car ls))))
              (else (traverse (cdr ls))))))
    (traverse *strong-cues*)))

; try-weak-cues: if there are no strong cues, this one will be used.  Searches through
;                *weak-cues*.
;
(define try-weak-cues
  (lambda (sentence context)
    (define traverse
      (lambda (ls)
        (cond ((null? ls) '())
              ((any-good-fragments? (cue-part (car ls)) sentence)
               (get-weak-cue context (cdar ls)))
              (else (traverse (cdr ls))))))
    (traverse *weak-cues*)))

; get-weak-cue: once try-weak-cues has found a keyword in the sentence, get-weak-cue is
;               called to check the context of the current sentence and return a response.
;
(define get-weak-cue
  (lambda (sentence foo)
    (define traverse
      (lambda (ls)
        (cond ((null? ls) '())
              ((any-good-fragments? (cue-part (car ls)) sentence)
               (select-any-from-list (response-part (car ls))))
              (else (traverse (cdr ls))))))
    (traverse foo)))

; any-good-fragments?: checks to see if a pattern appears in a given sentence.
;
(define any-good-fragments?
  (lambda (list-of-patterns sentence)
    (cond ((null? list-of-patterns) #f)
          ((subsequence? (car list-of-patterns) sentence) #t)
          (else (any-good-fragments? (cdr list-of-patterns) sentence)))))

; select-any-from-list: randomly select an element of a list.
;
(define select-any-from-list
  (lambda (ls)
    (list-ref ls (random (length ls)))))

; list-ref: returns the value of a list at the given index.
; 
(define list-ref
  (lambda (ls index)
    (cond ((null? ls) '())
          ((= index 0)(car ls))
          (else (list-ref (cdr ls)(- index 1))))))

; writeln: print a sentence with a new line at the end.
;
(define writeln
  (lambda (sentence)
    (write sentence)
    (newline)))

; hedge: a catch-all for phrases that do not contain cues or ends.  Uses *hedges*
;
(define hedge
  (lambda ()
    (select-any-from-list *hedges*)))

; wants-to-end?: checks to see if the user wants to end the session based on keywords.
;
(define wants-to-end?
  (lambda (sentence)
    (any-good-fragments? '((had enough) (done) (bye) (quit)) sentence)))

; wrap-it-up: prints a final message before the program ends.
;
(define wrap-it-up
  (lambda ()
    (writeln '(so long))))

; supsequence?: determines if the pattern appears in the list.
;
(define subsequence?
  (lambda (pattern ls)
    (cond ((null? pattern) #t)
          ((null? ls) #f)
          ((prefix? pattern ls) #t)
          (else (subsequence? pattern (cdr ls))))))

; prefix?: determines if pattern is the beginning of the list.
;
(define prefix?
  (lambda (pattern ls)
    (cond ((null? pattern) #t)
          ((null? ls) #f)
          ((equal? (car pattern) (car ls)) (prefix? (cdr pattern) (cdr ls)))
          (else #f))))

; cue-part: return the first element of a pair. 
;
(define cue-part
  (lambda (pair)
    (car pair)))

; response-part: return the first element of the end of pair.
;
(define response-part
  (lambda (pair)
    (cadr pair)))

; *weak-cues*: a list of cues, contexts, and replies that are nested.
;
(define *weak-cues*
  '( ( ((whos))
       (((first-base))
        ((thats right) (exactly) (you got it)
         (right on) (now youve got it)))
       (((second-base) (third-base))
        ((no whos on first) (whos on first) (first base))) )
     ( ((whats))
       (((first-base) (third-base))
        ((hes on second) (i told you whats on second)))
       (((second-base))
        ((right) (sure) (you got it right))) )
     ( ((whats the name))
       (((first-base) (third-base))
        ((no whats the name of the guy on second)
         (whats the name of the second baseman)))
       (((second-base))
        ((now youre talking) (you got it))))
     ))

; *strong-cues:* a list of cues and responses that are nested.
;
(define *strong-cues*
  '( ( ((the names) (their names) (the players))
       ((whos on first whats on second i dont know on third)
        (whats on second whos on first i dont know on third)) )
     
     ( ((suppose) (lets say) (assume))
       ((okay) (why not) (sure) (it could happen)) )
     
     ( ((i dont know))
       ((third base) (hes on third)) )
       
     ( ((what))
       ((second base) (hes on second)) )
       
     ( ((who))
       ((first base) (hes on first)) )
     ))

; *context-words*: a list of cues and contexts that are nested.
;
(define *context-words*
  '( ( ((first)) first-base )
     ( ((second)) second-base )
     ( ((third)) third-base )
     ))

; *hedges*: a list of phrases to say when no cues or ends are found.
;
(define *hedges*
  '( (it's like im telling you)
     (now calm down)
     (take it easy)
     (its elementary lou)
     (im trying to tell you)
     (but you asked)
     ))
