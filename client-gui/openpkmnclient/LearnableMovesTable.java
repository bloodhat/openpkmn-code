/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.io.Serializable;
import java.util.List;
import javax.swing.DefaultCellEditor;
import javax.swing.JComboBox;
import javax.swing.JTable;
import javax.swing.table.TableModel;

/**
 *
 * @author matt
 */
public class LearnableMovesTable extends JTable implements
        CreatablePkmnChangedListener, RulesTableUpdatedListener,
        MoveSelectionPolicyChangedListener, Serializable {

    public LearnableMovesTable() {
        super(new LearnableMovesTableModel());
        Object[] ppUpVals = {3, 2, 1, 0};
        JComboBox ppUpEditor = new JComboBox(ppUpVals);
        this.getColumnModel().getColumn(2).
                setCellEditor(new DefaultCellEditor(ppUpEditor));
        Object[] moveLearnVals = {"Auto"};
        JComboBox moveLearnEditor = new JComboBox(moveLearnVals);
        this.getColumnModel().getColumn(3).
                setCellEditor(new DefaultCellEditor(moveLearnEditor));
    }

    public List<Integer> getSelectedMoves() {
        return ((LearnableMovesTableModel) getModel()).getSelectedMoves();
    }

    @Override
    public void rulesTableUpdated(RulesTableUpdatedEvent e) {
        TableModel t = getModel();
        if (t != null) {
            ((LearnableMovesTableModel) t).changeRules(e.getRules());
        }
    }

    @Override
    public void creatablePkmnChanged(CreatablePkmnChangedEvent e) {
        TableModel t = getModel();
        if (t != null) {
            ((LearnableMovesTableModel) t).changePkmn(e.getId());
        }
    }

    @Override
    public void moveSelectionPolicyChanged(MoveSelectionPolicyChangedEvent e) {
        TableModel t = getModel();
        Policy p = e.getPolicy();
        if(p.equals(Policy.RANDOM)) {
            setEnabled(false);
        } else {
            setEnabled(true);
        }
        if (t != null) {
            ((LearnableMovesTableModel) t).changeSelectionPolicy(e.getPolicy());
        }
    }

}
