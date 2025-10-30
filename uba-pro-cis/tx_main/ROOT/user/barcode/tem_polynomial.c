/*******************************************************************************
* Project: CIS Bill Acceptor
* File: tem_polynomial.c
* Contents: polynomials used for error correction
*
*
*******************************************************************************/

#define EXT
#include "tem_global.c"


void initialize_polynomial(P_POLYNOMIAL const polynomial, const int value)
{
	int count = 0;

	for(count = 0; count < POLYNOMIAL_MAX_SIZE; count++)
	{
		polynomial->value[count] = value;
	}

	polynomial->length = 0;
}


void copy_polynomial(P_POLYNOMIAL const source, P_POLYNOMIAL const destination)
{
	int count = 0;

	for(count = 0; count < source->length; count++)
	{
		destination->value[count] = source->value[count];
	}

	destination->length = source->length;
}


void reverse_polynomial(P_POLYNOMIAL const polynomial)
{
	int count = 0;

	int temp = 0;

	for(count = 0; count < polynomial->length / 2; count++)
	{
		temp = polynomial->value[count];
		polynomial->value[count] = polynomial->value[polynomial->length - count - 1];
		polynomial->value[polynomial->length - count - 1] = temp;
	}
}


void remove_polynomial_zeros(P_POLYNOMIAL const polynomial)
{
	int count = 0;

	for(count = POLYNOMIAL_MAX_SIZE - 1; count >= 0; count--)
	{
		if(polynomial->value[count] != 0)
		{
			polynomial->length = count + 1;

			return;
		}
	}

	polynomial->length = 0;
}


void insert_front_of_polynomial(P_POLYNOMIAL const polynomial, const int value)
{
	int count = 0;

	for(count = polynomial->length; count >= 1; count--)
	{
		polynomial->value[count] = polynomial->value[count - 1];
	}

	polynomial->value[0] = value;

	polynomial->length++;
}


void insert_end_of_polynomial(P_POLYNOMIAL const polynomial, const int value)
{
	polynomial->value[polynomial->length] = value;

	polynomial->length++;
}
