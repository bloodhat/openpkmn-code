/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.util.EnumSet;
import javax.swing.table.AbstractTableModel;

/**
 *
 * @author matt
 */


    public class RulesTableModel extends AbstractTableModel {

        private String columnNames[] = {"Rule", "Mnemonic", "Enabled"};
        EnumSet<Rule> prefs;
        private Object data[][];

        public RulesTableModel() {
            Rule ps[] = Rule.values();
            data = new Object[ps.length][3];
            for (int i = 0; i < ps.length; i++) {
                Rule p = ps[i];
                data[i][0] = p.name;
                data[i][1] = p.code;
                data[i][2] = false;
            }
            prefs = EnumSet.noneOf(Rule.class);
        }

        @Override
        public int getRowCount() {
            return data.length;
        }

        @Override
        public int getColumnCount() {
            return columnNames.length;
        }

        @Override
        public Object getValueAt(int r, int c) {
            return data[r][c];
        }

        @Override
        public String getColumnName(int c) {
            return columnNames[c];
        }

        @Override
        public Class getColumnClass(int c) {
            return getValueAt(0, c).getClass();
        }

        @Override
        public boolean isCellEditable(int row, int col) {
            if (col == 2) {
                return true;
            } else {
                return false;
            }
        }

        public boolean setRules(int rules) {
            if (rules == -1) {
                return false;
            }
                prefs = Rule.getRuleSetFromValue(rules);
                for (Rule p : prefs) {
                    data[p.ordinal()][2] = true;
                }
                this.fireTableDataChanged();
                return true;
        }

        @Override
        public void setValueAt(Object value, int row, int col) {
            data[row][col] = value;
            if ((Boolean) value == true) {
                prefs.add(Rule.values()[row]);
            } else {
                prefs.remove(Rule.values()[row]);
            }
            fireTableCellUpdated(row, col);
        }

        public EnumSet<Rule> getSelectedRules() {
            return prefs;
        }
    }
