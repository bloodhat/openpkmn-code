package openpkmnclient;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author matt
 */
public class BattlePkmnTable extends JTable {
    public BattlePkmnTable() {
        this.setModel(new BattlePkmnTableModel());
    }

    public void load(List<BattlePkmnRosterItem> items) {
        ((BattlePkmnTableModel)getModel()).load(items);
    }

    public class BattlePkmnTableModel extends AbstractTableModel {
    private String[] columnNames = {
        "Name", "Level", "Status", "HP"};
    private Class[] columnClasses = {
        String.class, Integer.class, String.class, String.class };
    private HashMap<String, ArrayList> data;

    public BattlePkmnTableModel() {
        data = new HashMap<String, ArrayList>();
        for(int i = 0; i < columnNames.length; i++) {
            data.put(columnNames[i], new ArrayList());
        }
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

    public void load(List<BattlePkmnRosterItem> items) {
        data = new HashMap<String, ArrayList>();
        for(int i = 0; i < columnNames.length; i++) {
            data.put(columnNames[i], new ArrayList());
        }
        for(BattlePkmnRosterItem item : items) {
            data.get("Name").add(item.getNickname());
            data.get("Level").add(item.getLevel());
            data.get("Status").add(Lexicon.pkmnStatuses[item.getStatus()]);
            data.get("HP").add(item.getCurrentHP() + "/" + item.getMaxHP());
        }
        this.fireTableDataChanged();
    }
    }

}
