/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import javax.swing.table.AbstractTableModel;

/**
 *
 * @author matt
 */
public class AvailableTeamsTableModel extends AbstractTableModel {

    private String[] columnNames = {
        "ID", "Size", "Rules"};
    private Class[] columnClasses = {Integer.class, Integer.class,
        String.class};
    private HashMap<String, ArrayList> data;

    public AvailableTeamsTableModel() {
        data = new HashMap<String, ArrayList>();
        for (int i = 0; i < columnNames.length; i++) {
            data.put(columnNames[i], new ArrayList());
        }
    }

    @Override
    public int getColumnCount() {
        return columnNames.length;
    }

    @Override
    public int getRowCount() {
        return data.get("ID").size();
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

    public int getRowID(int row) {
        int id = (Integer) data.get("ID").get(row);
        return id;
    }

    public int getRowOfId(int id) {
        List<Integer> ids = data.get("ID");
        for(int i = 0; i < ids.size(); i++) {
            if(ids.get(i) == id) {
                return i;
            }
        }
        return -1;
    }

    public List<Integer> getRowMembers(int row) {
        List<Integer> members = (List<Integer>) data.get("Members").get(row);
        return members;
    }

    public String getRowRules(int row) {
        String rules = (String) data.get("Rules").get(row);
        return rules;
    }

    public void load(List<AvailableTeam> teams) {
        data = new HashMap<String, ArrayList>();
        for (int i = 0; i < columnNames.length; i++) {
            data.put(columnNames[i], new ArrayList());
        }
        data.put("Members", new ArrayList());
        for (AvailableTeam team : teams) {
            data.get("ID").add(team.getIndex());
            data.get("Size").add(team.getMembers().size());
            data.get("Rules").add(Rule.getCodeFromValue(team.getRules()));
            data.get("Members").add(team.getMembers());
        }
        this.fireTableDataChanged();
    }

    List<Integer> getSelectedIds() {
        List<Integer> ids = new ArrayList<Integer>();
        List<Boolean> isRecipientList = (List<Boolean>)data.get("Select");
        List<Integer> idList = (List<Integer>)data.get("ID");
        for(int i= 0; i < isRecipientList.size(); i++) {
            if(isRecipientList.get(i)) {
                ids.add(idList.get(i));
            }
        }
        return ids;
    }

}
