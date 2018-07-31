/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package openpkmnclient;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;

/**
 *
 * @author matt
 */
public class BattleMovesTable extends JTable {
    public BattleMovesTable() {
        this.setModel(new BattleMoveTableModel());
    }

    public void load(List<Move> moves) {
        ((BattleMoveTableModel)getModel()).load(moves);
    }

    public class BattleMoveTableModel extends AbstractTableModel {
    private String[] columnNames = {
        "Name", "PP"};
    private Class[] columnClasses = {
        String.class, Integer.class };
    private HashMap<String, ArrayList> data;

    public BattleMoveTableModel() {
        data = new HashMap<String, ArrayList>();
        for(int i = 0; i < columnNames.length; i++) {
            data.put(columnNames[i], new ArrayList());
        }
        data.put("id", new ArrayList());
    }


        @Override
    public int getColumnCount() {
        return columnNames.length;
    }

        @Override
    public int getRowCount() {
        return data.get("Name").size();
    }

    @Override
    public String getColumnName(int col) {
        return columnNames[col];
    }

        @Override
    public Object getValueAt(int row, int col) {
        return data.get(columnNames[col]).get(row);
    }

    @Override
    public Class getColumnClass(int c) {
        return columnClasses[c];
    }

    @Override
    public boolean isCellEditable(int row, int col) {
       return false;
    }

    public void load(List<Move> moves) {
        data = new HashMap<String, ArrayList>();
        for(int i = 0; i < columnNames.length; i++) {
            data.put(columnNames[i], new ArrayList());
        }
        for(Move move : moves) {
            data.get("Name").add(move.getName());
            data.get("PP").add(move.getPP());
        }
        this.fireTableDataChanged();
    }
    }
}
