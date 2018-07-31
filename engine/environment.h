#ifndef __OPENPKMN_ENVIRONMENT__
#define __OPENPKMN_ENVIRONMENT__

#define TYPE_MAX 16
#define MOVE_MAX 166
#define PKMN_MAX 152
#define NUM_EVOLVED 81

#define TYPELESS 0
#define FIRE 1
#define WATER 2
#define GRASS 3
#define ELECTRIC 4
#define ICE 5
#define PSYCHIC 6
#define NORMAL 7
#define FIGHTING 8
#define FLYING 9
#define GROUND 10
#define ROCK 11
#define BUG 12
#define POISON 13
#define GHOST 14
#define DRAGON 15

#define ATTACK 1
#define DEFENSE 2
#define SPEED 3
#define SPECIAL 4

enum statuses { NRM = 0,
		SLP = 8,
		PAR = 9,
		BRN = 10,
		PSN = 11,
		FRZ = 12,
		FNT = 13 };

extern const unsigned char stats[PKMN_MAX][5];
extern const unsigned char type[PKMN_MAX][2];

extern const int move_type[MOVE_MAX];
#endif
