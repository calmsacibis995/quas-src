/*	Awrite_word.c	1.1	86/07/20	*/

#include	"../tahoealign/align.h"

write_word (infop, word, where)
process_info	*infop;
long 		word;
struct oprnd 	*where;
/*
/*	Put the word at the given address in
/*	tahoe's memory.
/*	
/*	1. The least significant word is written.
/*
/**************************************************/
{
	register struct operand_des *look_at;

	look_at = &Table[opCODE].operand[last_operand];
	if (! (look_at->add_modes & NOVF))
		if (word > 0x7fff || word < -0x8000) overflow_1;	
	if (! (where->mode & W)) exception(infop, ILL_ADDRMOD);
	switch (where->mode & ADDFIELD)	/* Mask out R/W bits */
	{
	case Add:
		put_word (infop, word, where->address);
		break;
	case Dir:
		Replace (infop, where->reg_number, word);
		break;
	case SPmode: 
		where->mode = where->mode & ~SPmode | Add; 
		write_longword (infop, word, where);
		break;
	default:
		printf("Unknown destination in write_word (alignment code)\n");
	};
}	
