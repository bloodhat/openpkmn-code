/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

import java.util.ArrayList;
import java.util.Collection;
import java.util.EventListener;
import java.util.EventObject;
import openpkmnclient.MainController.InitialStatus;
import openpkmnclient.MainModel.MainModelStatusChangedEventListener.MainModelStatusChangedEvent;

/**
 *
 * @author matt
 */
public class MainModel {

    public enum Status {

        LOGGED_OUT("Logged out"),
        LOGGED_IN("Logged in"),
        WAITING("Waiting"),
        CHALLENGED("Challenged"),
        AWAITING_RESPONSE("Awaiting Response"),
        BATTLE_PENDING("Battle Pending"),
        BATTLING("Battling"),
        FINDING_BATTLE("Finding Battle"),
        FINDING_RANDOM("Finding Random Battle");
        public final String name;

        Status(String name) {
            this.name = name;
        }

        public String getDisplayName() {
            return name;
        }
    }
    private Status status = Status.LOGGED_OUT;
    private String userName;
    private final Collection<MainModelStatusChangedEventListener> statusChangeListeners;

    public MainModel() {
            statusChangeListeners =
                    new ArrayList<MainModelStatusChangedEventListener>();
        
                
    }
  public Status getStatus() {
        return status;
    }

    public void setUserName(String u) {
        userName = u;
    }

    public String getUserName() {
        return userName;
    }

    public void setStatus(Status newStatus, Object data) {
        status = newStatus;
        if(data != null) {
            setUserName(((InitialStatus)data).getUserName());
        }
        fireStatusChanged(status, data);
    }

    public synchronized void addStatusChangeListener(MainModelStatusChangedEventListener l) {
       statusChangeListeners.add(l);
    }

    public synchronized void removeStatusChangeListener(
            MainModelStatusChangedEventListener l) {
            statusChangeListeners.remove(l);
    }

    public synchronized void fireStatusChanged(Status newStatus, Object data) {
            for (MainModelStatusChangedEventListener l : statusChangeListeners) {
                l.mainModelStatusChanged(new MainModelStatusChangedEvent(this,
                        newStatus, data));
            }      
    }

    public interface MainModelStatusChangedEventListener extends EventListener {

        public class MainModelStatusChangedEvent extends EventObject {

            Status status;
            Object data;
            public MainModelStatusChangedEvent(Object source, Status status,
                    Object data) {
                super(source);
                this.status = status;
                this.data = data;
            }

            public Status getStatus() {
                return status;
            }

            public Object getData() {
                return data;
            }
        }

        public void mainModelStatusChanged(MainModelStatusChangedEvent e);
    }
}
