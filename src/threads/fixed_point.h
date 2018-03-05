// definition and implementation of fixed point numbers

#ifndef _FIXED_POINT_H
#define _FIXED_POINT_H

/* Basic definitions of fixed point. */
typedef int fixed_pt;

/* Using a shift of 14 */
#define FP_SHIFT (1 << 14)

/* Basic operations on fixed point type */

/* Convert a value to fixed-point value. */
#define FP_CONV(X) ((fixed_pt)(X * FP_SHIFT))

/* Arithmetic for both fixed point arguments. */
#define FP_ADD(X, Y) (X + Y)
#define FP_SUB(X, Y) (X - Y)
#define FP_MULT(X, Y) ((fixed_pt)(((int64_t) X) * Y / FP_SHIFT))
#define FP_DIV(X, Y) ((fixed_pt)(((int64_t) X) * FP_SHIFT / Y))

/* Arithmetic for first argument fixed, second int */
#define FP_INT_ADD(X, I) (X + (I * FP_SHIFT))
#define FP_INT_SUB(X, I) (X - (I * FP_SHIFT))
#define FP_INT_MULT(X, I) (X * I)
#define FP_INT_DIV(X, I) (X / I)

/* Integer part */
#define FP_INT_PART(X) (X / FP_SHIFT)

/* Get rounded integer of a fixed-point value. */
#define FP_FLOOR(X) (X >= 0 ? ((X + FP_SHIFT / 2) / FP_SHIFT) \
							: ((X - FP_SHIFT / 2) / FP_SHIFT))

#endif /* thread/fixed_point.h */
