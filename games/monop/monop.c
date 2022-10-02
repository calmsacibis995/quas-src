/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)monop.c	5.6 (Berkeley) 6/18/88";
#endif /* not lint */

# include	"monop.def"

/*
 *	This program implements a monopoly game
 */
main(ac, av)
reg int		ac;
reg char	*av[]; {


	srand(getpid());
	if (ac > 1) {
		if (!rest_f(av[1]))
			restore();
	}
	else {
		getplayers();
		init_players();
		init_monops();
	}
	num_luck = sizeof lucky_mes / sizeof (char *);
	init_decks();
	signal(2, quit);
	for (;;) {
		printf("\n%s (%d) (cash $%d) on %s\n", cur_p->name, player + 1,
			cur_p->money, board[cur_p->loc].name);
		printturn();
		force_morg();
		execute(getinp("-- Command: ", comlist));
	}
}
/*
 *	This routine gets the names of the players
 */
getplayers() {

	reg char	*sp;
	reg int		i, j;
	char		buf[257];

blew_it:
	for (;;) {
		if ((num_play=get_int("How many players? ")) <= 0 ||
		    num_play > MAX_PL)
			printf("Sorry. Number must range from 1 to 9\n");
		else
			break;
	}
	cur_p = play = (PLAY *) calloc(num_play, sizeof (PLAY));
	for (i = 0; i < num_play; i++) {
over:
		printf("Player %d's name: ", i + 1);
		for (sp = buf; (*sp=getchar()) != '\n'; sp++)
			continue;
		if (sp == buf)
			goto over;
		*sp++ = '\0';
		strcpy(name_list[i]=play[i].name=(char *)calloc(1,sp-buf),buf);
		play[i].money = 1500;
	}
	name_list[i++] = "done";
	name_list[i] = 0;
	for (i = 0; i < num_play; i++)
		for (j = i + 1; j < num_play; j++)
			if (strcasecmp(name_list[i], name_list[j]) == 0) {
				if (i != num_play - 1)
					printf("Hey!!! Some of those are IDENTICAL!!  Let's try that again....\n");
				else
					printf("\"done\" is a reserved word.  Please try again\n");
				for (i = 0; i < num_play; i++)
					cfree(play[i].name);
				cfree(play);
				goto blew_it;
			}
}
/*
 *	This routine figures out who goes first
 */
init_players() {

	reg int	i, rl, cur_max;
	bool	over;
	int	max_pl;

again:
	putchar('\n');
	for (cur_max = i = 0; i < num_play; i++) {
		printf("%s (%d) rolls %d\n", play[i].name, i+1, rl=roll(2, 6));
		if (rl > cur_max) {
			over = FALSE;
			cur_max = rl;
			max_pl = i;
		}
		else if (rl == cur_max)
			over++;
	}
	if (over) {
		printf("%d people rolled the same thing, so we'll try again\n",
		    over + 1);
		goto again;
	}
	player = max_pl;
	cur_p = &play[max_pl];
	printf("%s (%d) goes first\n", cur_p->name, max_pl + 1);
}
/*
 *	This routine initalizes the monopoly structures.
 */
init_monops() {

	reg MON	*mp;
	reg int	i;

	for (mp = mon; mp < &mon[N_MON]; mp++) {
		mp->name = mp->not_m;
		for (i = 0; i < mp->num_in; i++)
			mp->sq[i] = &board[mp->sqnums[i]];
	}
}
