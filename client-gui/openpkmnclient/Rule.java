/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.util.Map;
import java.util.HashMap;
import java.util.EnumSet;

public enum Rule {

    SLEEP_CLAUSE("Sleep Clause", "Z", 0x00000001),
    FREEZE_CLAUSE("Freeze Clause", "F", 0x00000002),
    NO_MEW("No Mew", "M", 0x00000004),
    NO_MEWTWO("No Mewtwo", "2", 0x00000008),
    NO_EVADE("No Evade-changing", "D", 0x00000010),
    NO_OHKO("No OHKOs", "O", 0x00000020),
    NO_GS("No Gold/Silver Tradeback Moves", "G", 0x00000040),
    NO_TM("No TMs/HMs", "T", 0x00000080),
    NO_ILLEGAL_MOVES("No Illegal Moves", "V", 0x00000100),
    NO_ILLEGAL_LEVELS("No Illegal Level", "L", 0x00000200),
    NO_ILLEGAL_STATS("No Illegal Stats", "A", 0x00000400),
    SPECIES_CLAUSE("Species Clause", "S", 0x00000800);

    static EnumSet<Rule> getUserToUserRules() {
        return EnumSet.of(SLEEP_CLAUSE, FREEZE_CLAUSE);
    }

    static EnumSet<Rule> getUserToTeamRules() {
        return EnumSet.of(NO_MEW, NO_MEWTWO, NO_EVADE, NO_OHKO, NO_GS, NO_TM,
                NO_ILLEGAL_MOVES, NO_ILLEGAL_LEVELS, NO_ILLEGAL_STATS,
                SPECIES_CLAUSE);
    }
    public final String name;
    public final String code;
    public final int value;

    Rule(String name, String code, int value) {
        this.name = name;
        this.code = code;
        this.value = value;
    }
    private static Map<String, Rule> codeTable = new HashMap<String, Rule>();
    private static Map<Integer, Rule> bitTable = new HashMap<Integer, Rule>();

    static {
        for (Rule p : Rule.values()) {
            codeTable.put(p.code, p);
            bitTable.put(p.value, p);
        }
    }

    public static Rule codeFor(String code) {
        return codeTable.get(code);
    }

    public static Rule bitFor(int bit) {
        return bitTable.get(bit);
    }

    public static int getRuleValueForSet(EnumSet<Rule> prefs) {
        int ruleValue = 0;
        for (Rule p : prefs) {
            ruleValue |= p.value;
        }
        return ruleValue;
    }

    public static int getRuleValueFromCodes(String codes) {
        int ruleValue = 0;
        if (codes != null) {
            if (codes.equals("-")) {
                return -1;
            }
            for (int i = 0; i < codes.length(); i++) {
                String s = "" + (codes.charAt(i));
                Rule r = codeFor(s);
                if (r != null) {
                    ruleValue |= r.value;
                } else {
                    System.err.println("cuidado!!! bad mf rule");
                }
            }
        } else {
            System.err.println("ok which wiseguy sent null rules!!!!");
            return -1;
        }
        return ruleValue;
    }

    public static EnumSet<Rule> getRuleSetFromValue(int value) {
        EnumSet e = EnumSet.noneOf(Rule.class);
        if (value == -1) {
            return null;
        }
        for (int i = 0; i < 32; i++) {
            int mask = 0x1 << i;
            if ((value & mask) == mask) {
                Rule r = bitFor(mask);
                if (r != null) {
                    e.add(r);
                } else {
                    System.err.println("cuidado!!! bad mf rule");
                }
            }
        }
        return e;
    }

    public static String getCodeFromValue(int value) {
        if (value == -1) {
            return "-";
        }
        String code = "";
        for (int i = 0; i < 32; i++) {
            int mask = 0x1 << i;
            if ((value & mask) == mask) {
                Rule r = bitFor(mask);
                if (r != null) {
                    code = code + r.code;
                } else {
                    System.err.println("cuidado!!! bad mf rule");
                }
            }
        }
        return code;
    }

    public static boolean rulesConcernPkmnChoice(EnumSet<Rule> rules) {
        return rules.contains(Rule.NO_MEW) || rules.contains(Rule.NO_MEW);
    }

    public static boolean rulesConernMoveChoice(EnumSet<Rule> rules) {
        return rules.contains(NO_EVADE) || rules.contains(NO_OHKO)
                || rules.contains(NO_GS) || rules.contains(NO_TM)
                || rules.contains(NO_ILLEGAL_MOVES);
    }
    public static final int defaultPrefs = 0x00000000;
}
