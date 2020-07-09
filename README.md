# lispy
`lispy` is a simple implementation of a modest subset of scheme. I started making it while I was following along with the book 
[Build Your Own Lisp](http://www.buildyourownlisp.com/). However, I departed from the book significantly, choosing C++ instead
of C for the implementation. The version of Lisp implemented in the book departs from traditional Lisps in some places in favor
of what the author believes is a simpler exposition, but I tried to stick to familiar Scheme constructs as much as possible.

The project is very much a work in progress -- it doesn't even have garbage collection now! As I have more time, I hope to work
on it more to add garbage collection and new language features. However, there's one thing you can do with it already -- you can
write the square-root procedure as described in the first chapter of [Structure and Interpretation of Computer Programs](https://mitpress.mit.edu/sites/default/files/sicp/index.html)!
(Internal definitions won't work yet, but for the moment you can write in a more explicit style)

Sample interaction:
```scheme
$ ./lispy 
Lispy Version 0.0.0.0.1
Press Ctrl+d to Exit

lispy> (define (avg x y) (/ (+ x y) 2))
(lambda (x y) ...)
lispy> (define (improve guess x) (avg guess (/ x guess)))
(lambda (guess x) ...)
lispy> (improve 1 2)
1.5
lispy> (define (good-enough? guess x) (< (abs (- (sqr guess) x)) 0.001))
(lambda (guess x) ...)
lispy> (good-enough? 1.5 2)
eval: undefined variable 'abs'
lispy> (define (abs x) (if (< x 0) (- x) x))
(lambda (x) ...)
lispy> (good-enough? 1.5 2)
eval: undefined variable 'sqr'
lispy> (define (sqr x) (* x x))
(lambda (x) ...)
lispy> (good-enough? 1.5 2)
#f
lispy> (define (sqrt-iter guess x) (if (good-enough? guess x) guess (sqrt-iter (improve guess x) x)))
(lambda (guess x) ...)
lispy> (sqrt-iter 1 2)
1.41422
lispy> (define (sqrt x) (sqrt-iter 1 x))
(lambda (x) ...)
lispy> (sqrt 2)
1.41422
lispy> 
```
