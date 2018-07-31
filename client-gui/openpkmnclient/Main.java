/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package openpkmnclient;

/**
 *
 * @author matt
 */
public class Main {

    /**
     * @param arguments the command line arguments
     */
    public static void main(String args[]) {
        final String arguments[];
        final TabbedMainWindow mainWindow;
        final MainController mainController;
        final MainModel mainModel;
        final BattleModel battleModel;

        arguments = args;

        mainWindow = new TabbedMainWindow();

        mainController = new MainController();

        mainModel = new MainModel();
        battleModel = new BattleModel();

         mainWindow.setController(mainController);
                    mainWindow.setModel(battleModel);
                    mainWindow.setModel(mainModel);
        java.awt.EventQueue.invokeLater(new Runnable() {

            @Override
            public void run() {
                System.err.println(arguments + " " + arguments.length);
                
                if (arguments.length == 4) {
                    MainController.InitialStatus initialStatus =
                            mainController.doLogin(new Credentials(arguments[0],
                            arguments[1], arguments[2],
                            Integer.parseInt(arguments[3])));
                    if (initialStatus != null) {
                        mainModel.setStatus(MainModel.Status.LOGGED_IN,
                                initialStatus);
                    } else {
                        mainModel.setStatus(MainModel.Status.LOGGED_OUT, null);
                    }
                } else {
                    mainModel.setStatus(MainModel.Status.LOGGED_OUT, null);
                }
            }
        });
    }
}
