#ifndef BIT_H
#define BIT_H

#define BitSet(arg, posn) ((arg) | (1L << (posn)))
#define BitClr(arg, posn) ((arg) & ~(1L << (posn)))
#define BitTst(arg, posn) BOOL((arg) & (1L << (posn)))
#define BitFlp(arg, posn) ((arg) ^ (1L << (posn)))

#endif
