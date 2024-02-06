# Linear Algebra (Core Library)

## What is in `linalg`?
### Classes

* Matrix
 /// Dets, Inverse, Tranpose, etc.
* Pure Math functions
 /// quadratic roots
 /// cubic roots
 /// quartic roots

## Cubic Roots

There is a great deal of help in *starting* to understand cubic formulas online. However a vast majority of these _do not_ do enough to walk the solutions all the way to the end and usually leave out the critical bit at the end of solving the 2,3 or 6 complex solutions and how they work.

* Mathologer: https://www.youtube.com/watch?v=N-KXStupwsc
* https://encyclopediaofmath.org/index.php?title=Cardano_formula
* An implementation (not complete) in "C": https://stackoverflow.com/questions/40439739/cube-root-of-a-number-c (this is mostly wrong).
* Wolfram (perhaps too much detail) https://mathworld.wolfram.com/CubicFormula.html

Usually they skip over the part where the cube root of a non-zero complex number has _three_ solutions and how to obtain them. However there is a page that (https://socratic.org/questions/how-do-i-find-the-cube-root-of-a-complex-number).

Now to get back to the Cubic Formula implementation...

Cardano's published formulation from Targalia from Ferro starts from:

    x^3 + px + q = 0

However most polynomials are in the form:

    ax^3 + bx^2 + cx + d = 0.

We can use these to make convert into Cardano's format:

    b /= a, c /= a, d /= a
    Q = (3.0*c - b*b)/9.0;
    R = (b*(9.0*c - (2.0*b*b)) - 27.0*d)/54.0;
    // the standard p & q are then
    p = Q / 3
    q = R / 2
    D = Q*Q*Q + R*R

This Q/R format has been pre-shifted by `-b/(3*a)`. This implementation then
relies on the `cbrt` for `std::complex` types I implemented which returns `3` roots.

I used the video by Mathologer to get the general formulation: https://www.youtube.com/watch?v=N-KXStupwsc

* @ 1:19 for complex cubic w/ a,b,d,c,e
* @ 2:56 for simplified cubic w/ p & q.
* @ 16:07 for definitions of p and q (don't know why it's not earlier)

But then I used a response on a C++ forum to determine Q/R values (http://www.cplusplus.com/forum/beginner/234717/).

There's 3 special cases:

1. `D > 0`
    The cubic crosses the y=0 at 1 point. There are 3 roots but 2 are imaginary and 1 real. In this case the cube roots can be kept in pairs.
2. `D == 0`
    There's a special optimization here for dropping D. 2 solutions or a special triple solution (inflection at origin).
3. `D < 0`
    There's 3 y=0 solutions, 1 real and 2 complex. The cube root pairs need to be summed as conjugate pairs.

## What to Add Next

A list of the features I would like to add.

* TODO (Linalg) Tensor implementation (more than just `precision[D][R][C]`)
* TODO (Linalg) SV Decomposition
* TODO (Linalg) EKF
* TODO (Linalg) SLAM
