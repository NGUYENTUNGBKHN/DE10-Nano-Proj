/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_rs256_correction.c
* Contents: reed solomon error correction
*
*
*******************************************************************************/
#include "string.h"

#define EXT
#include "tem_global.c"

#include "tem_rs256_correction.h"
#include "tem_polynomial.h"


int* rs_correction_logarithm;
int* rs_correction_exponent;

void clear_rs_correction_values()
{
	bill_info->rs_correction_codeword_error_found = FALSE;
	bill_info->rs_correction_error_count = 0;

	memset(&bill_info->rs_correction_syndromes, 0, sizeof(POLYNOMIAL));
	memset(&bill_info->rs_correction_error_locator, 0, sizeof(POLYNOMIAL));
	memset(&bill_info->rs_correction_error_position, 0, sizeof(POLYNOMIAL));
	memset(&bill_info->rs_correction_coefficient_position, 0, sizeof(POLYNOMIAL));
	memset(&bill_info->rs_correction_error_evaluator, 0, sizeof(POLYNOMIAL));
	memset(&bill_info->qr_barcode_result_temp.codewords_work, 0, sizeof(POLYNOMIAL));
	memset(&bill_info->qr_barcode_result_temp.block_message, 0, sizeof(POLYNOMIAL));
}

/* 2020-02-20 RS_BLOCK_COUNT */
int run_rs256_error_correction(P_POLYNOMIAL const message, const int correction_codeword_count,
	int* const logarithm_table, int* const exponent_table, int first_consecutive_root)
{
	int i;
	int j;
	int pos;
	P_POLYNOMIAL block_message = &bill_info->qr_barcode_result_temp.block_message;
	P_POLYNOMIAL work_message = &bill_info->qr_barcode_result_temp.codewords_work;
	int correction_count = bill_info->qr_barcode_result_temp.correction_codeword_count;
	int data_count = message->length - correction_count;
	const int rs_block_count = bill_info->qr_barcode_result_temp.rs_block_count;
	clear_rs_correction_values();

	bill_info->rs_correction_first_consecutive_root = first_consecutive_root;
	rs_correction_logarithm = logarithm_table;
	rs_correction_exponent = exponent_table;

	for( i = 0; i < rs_block_count; i++)
	{
		block_message->length = message->length;
		// load data block
		pos = i;
		for( j = 0; j < data_count; j++)
		{
			block_message->value[j] = message->value[pos];
			pos += rs_block_count;
		}
		// load error correction code block
		pos = data_count * rs_block_count + i;
		for( j = data_count; j < block_message->length ; j++)
		{
			block_message->value[j] = message->value[pos];
			pos += rs_block_count;
		}
		rs256_correction_find_syndromes(block_message, correction_codeword_count);

		if(bill_info->rs_correction_codeword_error_found == TRUE) //check if there are any errors
		{
			rs256_correction_find_error_locator(correction_codeword_count);

			//too many errors, cannot fix case
			if(bill_info->rs_correction_error_count * 2 > correction_codeword_count)
			{
				return FALSE;
			}

			rs256_correction_find_error_positions(block_message);

			//too many errors, cannot fix case
			if(bill_info->rs_correction_error_position.length != bill_info->rs_correction_error_count)
			{
				return FALSE;
			}

			rs256_correction_find_error_evaluator();
			rs256_correction_correct_errors(block_message);
		}
		memcpy(&work_message->value[work_message->length], &block_message->value[0], data_count * sizeof(block_message->value[0]));
		work_message->length += data_count;
	}
	memcpy(message, work_message, sizeof(POLYNOMIAL));

	return TRUE;
}

void rs256_correction_find_syndromes(P_POLYNOMIAL const message, const int correction_codeword_count)
{
	int count = 0;
	int temp = 0;

	initialize_polynomial(&bill_info->rs_correction_syndromes, 0);

	insert_front_of_polynomial(&bill_info->rs_correction_syndromes, 0);

	//introduce error for testing
	//message->value[0] = 1;
	//message->value[1] = 2;
	//message->value[2] = 3;
	//message->value[3] = 4;
	//message->value[4] = 5;
	//message->value[5] = 6;

	for(count = bill_info->rs_correction_first_consecutive_root;
		count < correction_codeword_count + bill_info->rs_correction_first_consecutive_root; count++)
	{
		temp = gf256_power(2, count);

		insert_end_of_polynomial(&bill_info->rs_correction_syndromes, gf256_evaluate_polynomial(message, temp));
	}

	//check if we need to do error correction
	for(count = 0; count < bill_info->rs_correction_syndromes.length; count++)
	{
		if(bill_info->rs_correction_syndromes.value[count] != 0)
		{
			bill_info->rs_correction_codeword_error_found = TRUE;

			break;
		}
	}
}


void rs256_correction_find_error_locator(const int correction_codeword_count)
{
	POLYNOMIAL old_locator;
	POLYNOMIAL temp1_locator;
	POLYNOMIAL temp2_locator;

	int count1 = 0;
	int count2 = 0;
	int delta = 0;
	int syndrome_index = 0;

	initialize_polynomial(&bill_info->rs_correction_error_locator, 0);
	initialize_polynomial(&old_locator, 0);
	initialize_polynomial(&temp1_locator, 0);
	initialize_polynomial(&temp2_locator, 0);

	insert_front_of_polynomial(&bill_info->rs_correction_error_locator, 1);
	insert_front_of_polynomial(&old_locator, 1);

	for(count1 = 0; count1 < correction_codeword_count; count1++)
	{
    	syndrome_index = count1 + 1;

		delta = bill_info->rs_correction_syndromes.value[syndrome_index];

		for(count2 = 1; count2 < bill_info->rs_correction_error_locator.length; count2++)
		{
			delta ^= gf256_multiply(bill_info->rs_correction_error_locator.value[bill_info->rs_correction_error_locator.length - count2 - 1],
				bill_info->rs_correction_syndromes.value[syndrome_index - count2]);
		}

		insert_end_of_polynomial(&old_locator, 0);

		if(delta != 0)
		{
			if(old_locator.length > bill_info->rs_correction_error_locator.length)
			{
				gf256_multiply_scalar(&old_locator, &temp1_locator, delta);
				gf256_multiply_scalar(&bill_info->rs_correction_error_locator, &old_locator, gf256_inverse(delta));
				copy_polynomial(&temp1_locator, &bill_info->rs_correction_error_locator);
			}

			gf256_multiply_scalar(&old_locator, &temp1_locator, delta);
			gf256_add_polynomials(&bill_info->rs_correction_error_locator, &temp1_locator, &temp2_locator);
			copy_polynomial(&temp2_locator, &bill_info->rs_correction_error_locator);
		}
	}

	//remove zeros from error locator
	remove_polynomial_zeros(&bill_info->rs_correction_error_locator);
	reverse_polynomial(&bill_info->rs_correction_error_locator);

	bill_info->rs_correction_error_count = bill_info->rs_correction_error_locator.length - 1;
}


void rs256_correction_find_error_positions(P_POLYNOMIAL const message)
{
	int count = 0;

	initialize_polynomial(&bill_info->rs_correction_error_position, 0);

	for(count = 0; count < message->length; count++)
	{
		if(gf256_evaluate_polynomial(&bill_info->rs_correction_error_locator, gf256_power(2, count)) == 0)
		{
			insert_end_of_polynomial(&bill_info->rs_correction_error_position, message->length - count - 1);
		}
	}

	initialize_polynomial(&bill_info->rs_correction_coefficient_position, 0);

	for(count = 0; count < bill_info->rs_correction_error_position.length; count++)
	{
		insert_end_of_polynomial(&bill_info->rs_correction_coefficient_position, message->length - 1 - bill_info->rs_correction_error_position.value[count]);
	}
}


void rs256_correction_find_error_evaluator()
{
	POLYNOMIAL reversed_syndromes;
	POLYNOMIAL reversed_error_evaluator;
	POLYNOMIAL mulp;
	POLYNOMIAL divisor;

	initialize_polynomial(&reversed_syndromes, 0);
	initialize_polynomial(&reversed_error_evaluator, 0);
	initialize_polynomial(&mulp, 0);
	initialize_polynomial(&divisor, 0);
	initialize_polynomial(&bill_info->rs_correction_error_evaluator, 0);

	copy_polynomial(&bill_info->rs_correction_syndromes, &reversed_syndromes);
	reverse_polynomial(&reversed_syndromes);

	divisor.value[0] = 1;
	divisor.length = bill_info->rs_correction_error_locator.length + 1;

	reverse_polynomial(&bill_info->rs_correction_error_locator);

	gf256_multiply_polynomials(&reversed_syndromes, &bill_info->rs_correction_error_locator, &mulp);
	gf256_divide_polynomials(&mulp, &divisor, &reversed_error_evaluator);

	copy_polynomial(&reversed_error_evaluator, &bill_info->rs_correction_error_evaluator);
	reverse_polynomial(&bill_info->rs_correction_error_evaluator);
}


void rs256_correction_correct_errors(P_POLYNOMIAL const message)
{
	int count1 = 0;
	int count2 = 0;

	int Xi_inv = 0;
	int err_loc_prime = 0;
	int y = 0;
	int temp_int = 0;

	POLYNOMIAL X;
	POLYNOMIAL E;
	POLYNOMIAL err_loc_prime_temp;
	POLYNOMIAL corrected;

	initialize_polynomial(&X, 0);
	initialize_polynomial(&E, 0);
	initialize_polynomial(&err_loc_prime_temp, 0);
	initialize_polynomial(&corrected, 0);

	for(count1 = 0; count1 < bill_info->rs_correction_coefficient_position.length; count1++)
	{
		int l = bill_info->rs_correction_coefficient_position.value[count1] - 255;
		insert_end_of_polynomial(&X, gf256_power(2, l));
	}

	E.length = message->length;

	reverse_polynomial(&bill_info->rs_correction_error_evaluator);

	for(count1 = 0; count1 < X.length; count1++)
	{
		Xi_inv = gf256_inverse(X.value[count1]);

		err_loc_prime_temp.length = 0;

		for(count2 = 0; count2 < X.length; count2++)
		{
			if(count1 != count2)
			{
				insert_end_of_polynomial(&err_loc_prime_temp, 1 ^ gf256_multiply(Xi_inv, X.value[count2]));
			}
		}

		err_loc_prime = 1;

		for(count2 = 0; count2 < err_loc_prime_temp.length; count2++)
		{
			err_loc_prime = gf256_multiply(err_loc_prime, err_loc_prime_temp.value[count2]);
		}

		y = gf256_evaluate_polynomial(&bill_info->rs_correction_error_evaluator, Xi_inv);

		temp_int = gf256_power(X.value[count1], 1 - bill_info->rs_correction_first_consecutive_root);

		y = gf256_multiply(temp_int, y);

		E.value[bill_info->rs_correction_error_position.value[count1]] = gf256_divide(y, err_loc_prime);
	}

	gf256_add_polynomials(message, &E, &corrected);
	copy_polynomial(&corrected, message);
}


void gf256_add_polynomials(P_POLYNOMIAL const source1, P_POLYNOMIAL const source2, P_POLYNOMIAL const destination)
{
	int count = 0;

	initialize_polynomial(destination, 0);

	if(source1->length > source2->length)
	{
		destination->length = source1->length;
	}
	else
	{
		destination->length = source2->length;
	}

	for(count = 0; count < source1->length; count++)
	{
		destination->value[count + destination->length - source1->length] = source1->value[count];
	}

	for(count = 0; count < source2->length; count++)
	{
		destination->value[count + destination->length - source2->length] ^= source2->value[count];
	}
}


void gf256_multiply_polynomials(P_POLYNOMIAL const source1, P_POLYNOMIAL const source2, P_POLYNOMIAL const destination)
{
	int count1 = 0;
	int count2 = 0;

	initialize_polynomial(destination, 0);

	destination->length = source1->length + source2->length - 1;

	for(count2 = 0; count2 < source2->length; count2++)
	{
		for(count1 = 0; count1 < source1->length; count1++)
		{
			destination->value[count1 + count2] ^= gf256_multiply(source1->value[count1], source2->value[count2]);
		}
	}
}


//polynomial in ascending order
void gf256_divide_polynomials(P_POLYNOMIAL const source1, P_POLYNOMIAL const source2, P_POLYNOMIAL const destination)
{
	int count1 = 0;
	int count2 = 0;

	int coefficient = 0;

	const int size = source1->length - (source2->length - 1);

	copy_polynomial(source1, destination);

	for(count1 = 0; count1 < size; count1++)
	{
		coefficient = destination->value[count1];

		if(coefficient != 0)
		{
			for(count2 = 1; count2 < source2->length; count2++)
			{
				if(source2->value[count2] != 0)
				{
					destination->value[count1 + count2] ^= gf256_multiply(source2->value[count2], coefficient);
				}
			}
		}
	}

	//only keep the remainder
	for(count1 = 0; count1 < destination->length - size; count1++)
	{
		destination->value[count1] = destination->value[count1 + size];
	}

	destination->length -= size;
}


int gf256_multiply(const int value1, const int value2)
{
	if(value1 == 0 || value2 == 0)
	{
		return 0;
	}

	return rs_correction_exponent[(rs_correction_logarithm[value1] + rs_correction_logarithm[value2]) % 255];
}


int gf256_divide(const int value1, const int value2)
{
	if(value1 == 0)
	{
		return 0;
	}

	return rs_correction_exponent[(rs_correction_logarithm[value1] + 255 - rs_correction_logarithm[value2]) % 255];
}


void gf256_multiply_scalar(P_POLYNOMIAL const input, P_POLYNOMIAL const output, const int scalar)
{
	int count = 0;

	for(count = 0; count < input->length; count++)
	{
		output->value[count] = gf256_multiply(input->value[count], scalar);
	}

	output->length = input->length;
}


int gf256_power(const int base, const int power)
{
	int temp = rs_correction_logarithm[base];
	temp *= power;
	temp %= 255;

	if(temp < 0)
	{
		temp += 255;
	}

	return rs_correction_exponent[temp];
}


int gf256_inverse(const int value)
{
	return rs_correction_exponent[255 - rs_correction_logarithm[value]];
}


//polynomial in descending order
int gf256_evaluate_polynomial(P_POLYNOMIAL const polynomial, const int value)
{
	int result = polynomial->value[0];

	int count = 0;

	for(count = 1; count < polynomial->length; count++)
	{
		result = gf256_multiply(result, value) ^ polynomial->value[count];
	}

	return result;
}


void gf256_find_generator_polynomial(P_POLYNOMIAL const polynomial, const int length)
{
	int count1 = 0;
	int count2 = 0;

	initialize_polynomial(polynomial, 0);
	insert_end_of_polynomial(polynomial, 1);

	polynomial->length = length + 1;

	for(count1 = 0; count1 <= length; count1++)
	{
		for(count2 = count1; count2 >= 0; count2--)
		{
			polynomial->value[count2] = gf256_multiply(polynomial->value[count2], gf256_power(2, count1));
			if(count2 > 0)
			{
				polynomial->value[count2] ^= polynomial->value[count2 - 1];
			}
		}
	}
}


