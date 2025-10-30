/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_rs256_correction.h
* Contents: reed solomon error correction
*
*
*******************************************************************************/
#ifndef TEM_RS256_CORRECTION_H
#define TEM_RS256_CORRECTION_H

int run_rs256_error_correction(P_POLYNOMIAL const message, const int correction_codeword_count,
	int* const logarithm_table, int* const exponent_table, int first_consecutive_root);
void rs256_correction_find_syndromes(P_POLYNOMIAL const message, const int correction_codeword_count);
void rs256_correction_find_error_locator(const int correction_codeword_count);
void rs256_correction_find_error_positions(P_POLYNOMIAL const message);
void rs256_correction_find_error_evaluator(void);
void rs256_correction_correct_errors(P_POLYNOMIAL const message);

void gf256_add_polynomials(P_POLYNOMIAL const source1, P_POLYNOMIAL const source2, P_POLYNOMIAL const destination);
void gf256_multiply_polynomials(P_POLYNOMIAL const source1, P_POLYNOMIAL const source2, P_POLYNOMIAL const destination);
void gf256_divide_polynomials(P_POLYNOMIAL const source1, P_POLYNOMIAL const source2, P_POLYNOMIAL const destination);
int gf256_multiply(const int value1, const int value2);
int gf256_divide(const int value1, const int value2);
void gf256_multiply_scalar(P_POLYNOMIAL const input, P_POLYNOMIAL const output, const int scalar);
int gf256_power(const int base, const int power);
int gf256_inverse(const int value);
int gf256_evaluate_polynomial(P_POLYNOMIAL const polynomial, const int value);
void find_generator_polynomial(P_POLYNOMIAL const polynomial, const int length);

#endif
