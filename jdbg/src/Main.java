public class Main {

    public static void main(String[] args) {
        javax.swing.SwingUtilities.invokeLater(() -> {
            try {
                new Gui();
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        });
    }
}
