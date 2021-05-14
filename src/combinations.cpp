
#include "combinations.h"

int gen_comb_rep_lex_init(unsigned char *vector, const unsigned char n, const unsigned char k)
{
    int j; //index

           //test for special cases
    if (n == 0 || k == 0)
        return(GEN_EMPTY);

    //initialize: vector[0, ..., k - 1] are zero
    for (j = 0; j < k; j++)
        vector[j] = 0;

    return(GEN_NEXT);
}

int gen_comb_rep_lex_next(unsigned char *vector, const unsigned char n, const unsigned char k)
{
    int j; //index

           //easy case, increase rightmost element
    if (vector[k - 1] < n - 1)
    {
        vector[k - 1]++;
        return(GEN_NEXT);
    }

    //find rightmost element to increase
    for (j = k - 2; j >= 0; j--)
        if (vector[j] != n - 1)
            break;

    //terminate if all elements are n - 1
    if (j < 0)
        return(GEN_TERM);

    //increase
    vector[j]++;

    //set right-hand elements
    for (j += 1; j < k; j++)
        vector[j] = vector[j - 1];

    return(GEN_NEXT);
}

int gen_perm_rep_lex_init(const unsigned char n)
{
    //test for special cases
    if (n == 0)
        return(GEN_EMPTY);

    //initialize: vector must be initialized by the calling process

    return(GEN_NEXT);
}

int gen_perm_rep_lex_next(unsigned char *vector, const unsigned char n)
{
    int j = n - 2; //index
    int i = n - 1; //help index
    int temp;      //auxiliary element

                   //find rightmost element to increase
    while (j >= 0)
    {
        if (vector[j] < vector[j + 1])
            break;

        j--;
    }

    //terminate if all elements are in decreasing order
    if (j < 0)
        return(GEN_TERM);

    //find i
    while (vector[i] <= vector[j])
        i--;

    //increase (swap)
    temp = vector[j];
    vector[j] = vector[i];
    vector[i] = temp;

    //reverse right-hand elements
    for (j += 1, i = n - 1; j < i; j++, i--)
    {
        temp = vector[j];
        vector[j] = vector[i];
        vector[i] = temp;
    }

    return(GEN_NEXT);
}

int gen_vari_rep_colex_init(unsigned char *vector, const unsigned char m, const unsigned char n)
{
    int j; //index

           //test for special cases
    if (m == 0 || n == 0)
        return(GEN_EMPTY);

    //initialize: vector[0, ..., n - 1] are zero
    for (j = 0; j < n; j++)
        vector[j] = 0;

    return(GEN_NEXT);
}

int gen_vari_rep_colex_next(unsigned char *vector, const unsigned char m, const unsigned char n)
{
    int j; //index

           //easy case, increase leftmost element
    if (vector[0] < m)
    {
        vector[0]++;
        return(GEN_NEXT);
    }

    //find leftmost element to increase and reset left-hand elements
    for (j = 1; j < n; j++)
    {
        vector[j - 1] = 0;

        if (vector[j] < m)
            break;
    }

    //terminate if all elements are m - 1
    if (j == n)
        return(GEN_TERM);

    //increase
    vector[j]++;

    return(GEN_NEXT);
}
