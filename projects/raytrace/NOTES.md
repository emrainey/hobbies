# Overlaps

This table is truth table for how many maximal intersection points with a line are possible with different objects when put into different Overlaps.

| Types       | Sphere    | Torus | Wall        |
|-------------|-----------|-------|-------------|
| None        | 2         | 4     | 2           |
| Inclusive   | 2 (lens)  | 4 (arc?) | 2 (column)  |
| Exclusive   | 4 (inverted lens) | 8 (???) | 4 (inverted column) |
| Subtractive | 4 (bowl)  | 8 (hollow donut) | 4 (slitted wall) |
| Additive    | 4 (bubbles) | 8 (???)      | 4 (joined wall) |

This seems to follow a pattern:

* Inclusive = base type
* others = 2 x base type

For different objects

| Types       | Sphere + Wall |
|-------------|---------------|
| None        | 2, 2          |
| Inclusive   | 2 (sphere slice) |
| Exclusive   | 4             |
| Subtractive | 4 (sliced sphere) |
| Additive    | 4             |
