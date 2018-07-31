/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.EnumSet;
import javax.swing.JTable;
import javax.swing.event.TableModelEvent;
import javax.swing.table.TableModel;
import openpkmnclient.RulesTableUpdatedListener.RulesTableUpdatedEvent;
/**
 *
 * @author matt
 */
public class RulesTable extends JTable implements Serializable {

    private Collection<RulesTableUpdatedListener> listeners;

    public RulesTable() {
        super(new RulesTableModel());
         listeners = new ArrayList<RulesTableUpdatedListener>();
    }

    public void setRules(int rules) {
        ((RulesTableModel) getModel()).setRules(rules);
    }

    public EnumSet<Rule> getSelectedRules() {
        return ((RulesTableModel) getModel()).getSelectedRules();
    }

    public void addRulesTableUpdatedListener(RulesTableUpdatedListener l) {
        listeners.add(l);
    }

    public void removeRulesTableUpdatedListener(RulesTableUpdatedListener l) {
        listeners.remove(l);
    }

    private void fireRulesTableUpdated(EnumSet<Rule> newRules) {
        /* Guard against informing listeners during construction */
        if(listeners != null) {
            for (RulesTableUpdatedListener l : listeners) {
                l.rulesTableUpdated(new RulesTableUpdatedEvent(this, newRules));
            }
        }
    }

    // Getters and setters allow this to be a Java Bean
    public Collection<RulesTableUpdatedListener> getListeners() {
        return listeners;
    }

    public void setListeners(final Collection<RulesTableUpdatedListener> listeners) {
        this.listeners = listeners;
    }

    @Override
    public void tableChanged(TableModelEvent e) {
        super.tableChanged(e);
        TableModel t = getModel();
        /* Guard against the event where setModel is telling us that the table
         * has changed before the model has actually been set */
        if(t != null) {
            fireRulesTableUpdated(((RulesTableModel) t).getSelectedRules());
        }
    }

}

