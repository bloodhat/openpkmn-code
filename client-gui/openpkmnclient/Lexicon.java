/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

/**
 *
 * @author matt
 */
public class Lexicon {

    /* inline symbols
    @u the user's name
    @p the user's pkmn's nickname
    @o the opponent's pkmn's species name
    @m the move used as specified by the next message
    @i the integer value as specified by the next message
     */

    /* this is send over as 0xffff */
    final static int END_EVENTS = ((Short.MAX_VALUE + 1) * 2) - 1;
//    static final String[] userStatuses = {
//        "Offline", "Logged-in", "Waiting", "Challenged",
//        "Awaiting Response", "Battle Pending", "Battling"
//    };
    static final String[] eventMessages = {
        "@u sent out @p!",
        "@p used @m!",
        "@p's attack missed!",
        "But it failed!",
        "It's not very effective...",
        "It's super effective!",
        "It doesn't affect @o!",
        "@p flew up high!",
        "@p dug a hole",
        "@p lowered its head!",
        "Critical hit!",
        "Hit @i times!",
        "@p's building energy!",
        "@p's substitute broke!",
        "@p's rage is building",
        "@p's attack rose!",
        "@p's attack greatly rose!",
        "@p's defense rose!",
        "@p's defense greatly rose!",
        "@p's speed rose!",
        "@p's speed greatly rose!",
        "@p's special rose!",
        "@p's special greatly rose!",
        "@p's evade rose!",
        "@p's getting pumped!",
        "@p's attack fell!",
        "@p's attack greatly fell!",
        "@p's defense fell!",
        "@p's defense greatly fell!",
        "@p's speed fell!",
        "@p's speed greatly fell!",
        "@p's special fell!",
        "@p's special greatly fell!",
        "@p's accuracy fell!",
        "@p's hurt by the burn!",
        "@p's hurt by poison!",
        "Leech seed saps @p!",
        "@p fell asleep!",
        "@p was paralyzed!",
        "@p was burned!",
        "@p was poisoned!",
        "@p was badly poisoned!",
        "@p was frozen!",
        "@p became confused!",
        "@p was seeded!",
        "All status changes were eliminated!",
        "@p's protected against special attacks!",
        "@p gained armor!",
        "@p's shrouded in mist!",
        "@p transformed into @o!",
        "Converted type to @o's",
        "@p learned @m!",
        "@p regained health!",
        "@p fell asleep and became healthy!",
        "It created a substitute!",
        "@p woke up!",
        "@p is fast asleep!",
        "@p is frozen solid!",
        "@p's fully paralyzed!",
        "Fire defrosted @p!",
        "@p flinched!",
        "@p's @m was disabled!",
        "@p is confused!",
        "It hurt itslef in its confusion!",
        "@p must recharge!",
        "@p can't move!",
        "@p's thrashing about!",
        "@p's hit by recoil!",
        "@p's confused no more!",
        "@p's disabled no more!",
        "@p unleashed energy!",
        "@p's attack rose!",
        "@p fainted!",
        "@p sucked HP from @o",
        "@p's attack continues!",
	"@u won!",
	"@u lost...",
	"Tie...",
	"@u ran away...",
	"",
	"",
	"@i damage",
	"@p is at @i HP",
        "@p's substitute is at @i HP",
        "@m is disabled!",
        "@p made a whirlwind!",
        "@p is glowing!",
        "@p is gathering sunlight!"
    };
    
    static final String[] moveNames = {"NONE", "ABSORB",
        "ACID ARMOR", "ACID",
        "AGILITY", "AMNESIA",
        "AURORA BEAM", "BARRAGE",
        "BARRIER", "BIDE",
        "BIND", "BITE",
        "BLIZZARD", "BODY SLAM",
        "BONE CLUB", "BONEMERANG",
        "BUBBLE", "BUBBLEBEAM",
        "CLAMP", "COMET PUNCH",
        "CONFUSE RAY", "CONFUSION",
        "CONSTRICT", "CONVERSION",
        "COUNTER", "CRABHAMMER",
        "CUT", "DEFENSE CURL",
        "DIG", "DISABLE",
        "DIZZY PUNCH", "DOUBLE KICK",
        "DOUBLE TEAM", "DOUBLE-EDGE",
        "DOUBLESLAP", "DRAGON RAGE",
        "DREAM EATER", "DRILL PECK",
        "EARTHQUAKE", "EGG BOMB",
        "EMBER", "EXPLOSION",
        "FIRE BLAST", "FIRE PUNCH",
        "FIRE SPIN", "FISSURE",
        "FLAMETHROWER", "FLASH",
        "FLY", "FOCUS ENERGY",
        "FURY ATTACK", "FURY SWIPES",
        "GLARE", "GROWL", "GROWTH",
        "GUILLOTINE", "GUST", "HARDEN",
        "HAZE", "HEADBUTT",
        "HI JUMP KICK", "HORN ATTACK",
        "HORN DRILL", "HYDRO PUMP",
        "HYPER BEAM", "HYPER FANG",
        "HYPNOSIS", "ICE BEAM",
        "ICE PUNCH", "JUMP KICK",
        "KARATE CHOP", "KINESIS",
        "LEECH LIFE", "LEECH SEED",
        "LEER", "LICK",
        "LIGHT SCREEN", "LOVELY KISS",
        "LOW KICK", "MEDITATE",
        "MEGA DRAIN", "MEGA KICK",
        "MEGA PUNCH", "METRONOME",
        "MIMIC", "MINIMIZE",
        "MIRROR MOVE", "MIST",
        "NIGHT SHADE", "PAY DAY",
        "PECK", "PETAL DANCE",
        "PIN MISSILE", "POISON GAS",
        "POISON STING", "POISONPOWDER",
        "POUND", "PSYBEAM", "PSYCHIC",
        "PSYWAVE", "QUICK ATTACK",
        "RAGE", "RAZOR LEAF",
        "RAZOR WIND", "RECOVER",
        "REFLECT", "REST",
        "ROAR", "ROCK SLIDE",
        "ROCK THROW", "ROLLING KICK",
        "SAND-ATTACK", "SCRATCH",
        "SCREECH", "SEISMIC TOSS",
        "SELFDESTRUCT", "SHARPEN",
        "SING", "SKULL BASH",
        "SKY ATTACK", "SLAM",
        "SLASH", "SLEEP POWDER",
        "SLUDGE", "SMOG",
        "SMOKESCREEN", "SOFTBOILED",
        "SOLARBEAM", "SONICBOOM",
        "SPIKE CANNON", "SPLASH",
        "SPORE", "STOMP", "STRENGTH",
        "STRING SHOT", "STRUGGLE",
        "STUN SPORE", "SUBMISSION",
        "SUBSTITUTE", "SUPER FANG",
        "SUPERSONIC", "SURF", "SWIFT",
        "SWORDS DANCE", "TACKLE",
        "TAIL WHIP", "TAKE DOWN",
        "TELEPORT", "THRASH",
        "THUNDER WAVE", "THUNDER",
        "THUNDERBOLT", "THUNDERPUNCH",
        "THUNDERSHOCK", "TOXIC",
        "TRANSFORM", "TRI ATTACK",
        "TWINEEDLE", "VICEGRIP",
        "VINE WHIP", "WATER GUN",
        "WATERFALL", "WHIRLWIND",
        "WING ATTACK", "WITHDRAW",
        "WRAP"
    };
    static final String[] pkmnNames = {"NONE",
        "BULBASAUR", "IVYSAUR", "VENUSAUR",
        "CHARMANDER", "CHARMELEON", "CHARIZARD",
        "SQUIRTLE", "WARTORTLE", "BLASTOISE",
        "CATERPIE", "METAPOD", "BUTTERFREE",
        "WEEDLE", "KAKUNA", "BEEDRILL",
        "PIDGEY", "PIDGEOTTO", "PIDGEOT",
        "RATTATA", "RATICATE", "SPEAROW",
        "FEAROW", "EKANS", "ARBOK",
        "PIKACHU", "RAICHU", "SANDSHREW",
        "SANDSLASH", "NIDORAN FEMALE", "NIDORINA",
        "NIDOQUEEN", "NIDORAN MALE", "NIDORINO",
        "NIDOKING", "CLEFAIRY", "CLEFABLE",
        "VULPIX", "NINETALES", "JIGGLYPUFF",
        "WIGGLYTUFF", "ZUBAT", "GOLBAT",
        "ODDISH", "GLOOM", "VILEPLUME",
        "PARAS", "PARASECT", "VENONAT",
        "VENOMOTH", "DIGLETT", "DUGTRIO",
        "MEOWTH", "PERSIAN", "PSYDUCK",
        "GOLDUCK", "MANKEY", "PRIMEAPE",
        "GROWLITHE", "ARCANINE", "POLIWAG",
        "POLIWHIRL", "POLIWRATH", "ABRA",
        "KADABRA", "ALAKAZAM", "MACHOP",
        "MACHOKE", "MACHAMP", "BELLSPROUT",
        "WEEPINBELL", "VICTREEBEL", "TENTACOOL",
        "TENTACRUEL", "GEODUDE", "GRAVELER",
        "GOLEM", "PONYTA", "RAPIDASH",
        "SLOWPOKE", "SLOWBRO", "MAGNEMITE",
        "MAGNETON", "FARFETCH'D", "DODUO",
        "DODRIO", "SEEL", "DEWGONG",
        "GRIMER", "MUK", "SHELLDER",
        "CLOYSTER", "GASTLY", "HAUNTER",
        "GENGAR", "ONIX", "DROWZEE",
        "HYPNO", "KRABBY", "KINGLER",
        "VOLTROB", "ELECTRODE", "EXEGGCUTE",
        "EXEGGUTOR", "CUBONE", "MAROWAK",
        "HITMONLEE", "HITMONCHAN", "LICKITUNG",
        "KOFFING", "WEEZING", "RHYHORN",
        "RHYDON", "CHANSEY", "TANGELA",
        "KANGASKHAN", "HORSEA", "SEADRA",
        "GOLDEEN", "SEAKING", "STARYU",
        "STARMIE", "MR. MIME", "SCYTHER",
        "JYNX", "ELECTABUZZ", "MAGMAR",
        "PINSIR", "TAUROS", "MAGIKARP",
        "GYARADOS", "LAPRAS", "DITTO",
        "EEVEE", "VAPOREON", "JOLTEON",
        "FLAREON", "PORYGON", "OMANYTE",
        "OMASTAR", "KABUTO", "KABUTOPS",
        "AERODACTYL", "SNORLAX", "ARTICUNO",
        "ZAPDOS", "MOLTRES", "DRATINI",
        "DRAGONAIR", "DRAGONITE", "MEWTWO",
        "MEW"
    };

    /* we only know that sleep can be set server side any value from
    1-8, so we be safe */
    static final String[] pkmnStatuses = {"NRM", "SLP", "SLP", "SLP", "SLP",
        "SLP", "SLP", "SLP", "SLP", "PAR", "BRN", "PSN", "FRZ", "FNT"
    };
}

