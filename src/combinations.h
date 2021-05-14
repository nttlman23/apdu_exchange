#ifndef COMBINATIONS_H
#define COMBINATIONS_H

#define GEN_NEXT		0
#define GEN_TERM		1
#define GEN_EMPTY		-1

int gen_comb_rep_lex_init(unsigned char *vector, const unsigned char n, const unsigned char k);
int gen_comb_rep_lex_next(unsigned char *vector, const unsigned char n, const unsigned char k);
int gen_perm_rep_lex_init(const unsigned char n);
int gen_perm_rep_lex_next(unsigned char *vector, const unsigned char n);
int gen_vari_rep_colex_init(unsigned char *vector, const unsigned char m, const unsigned char n);
int gen_vari_rep_colex_next(unsigned char *vector, const unsigned char m, const unsigned char n);

#endif // COMBINATIONS_H
