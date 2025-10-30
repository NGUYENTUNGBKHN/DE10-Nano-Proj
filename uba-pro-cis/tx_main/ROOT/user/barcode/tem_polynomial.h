/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_polynomial.h
* Contents: polynomials used for error correction
*
*
*******************************************************************************/
#ifndef TEM_POLYNOMIAL_H
#define TEM_POLYNOMIAL_H

void initialize_polynomial(P_POLYNOMIAL const polynomial, const int value);
void copy_polynomial(P_POLYNOMIAL const source, P_POLYNOMIAL const destination);
void reverse_polynomial(P_POLYNOMIAL const polynomial);
void remove_polynomial_zeros(P_POLYNOMIAL const polynomial);
void insert_front_of_polynomial(P_POLYNOMIAL const polynomial, const int value);
void insert_end_of_polynomial(P_POLYNOMIAL const polynomial, const int value);

#endif
