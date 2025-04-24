import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.ArrayList;

public class Gui extends JFrame  implements KeyListener, MouseWheelListener {
    private VisualPanel visualPanel;
    private JButton zeroButton;
    private JButton averageButton;
    private Robot robot;

    record Pos(double x, double y){}
    private final ArrayList<Pos> maps = new ArrayList<>();

    public Gui() throws SocketException, UnknownHostException {
        robot = new Robot();

        setTitle("Wrobot");
        setSize(1200, 600);
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        setLayout(new BorderLayout());

        visualPanel = new VisualPanel();
        add(visualPanel, BorderLayout.CENTER);

        zeroButton = new JButton("Zero");
        zeroButton.addActionListener(e -> {
            robot.sendZero().error(ex -> ex.printStackTrace());
            maps.clear();
        });
        zeroButton.setFocusable(false);
        add(zeroButton, BorderLayout.SOUTH);

        averageButton = new JButton("Find Average Angle");
        averageButton.addActionListener( e -> averageAngle());
        averageButton.setFocusable(false);
        add(averageButton, BorderLayout.NORTH);


        setFocusable(true);
        addKeyListener(this);
        addMouseWheelListener(this);
        add(new PidPanel(robot), BorderLayout.EAST);

        setVisible(true);

        new Thread(() -> {
            while(true){
                try{
                    visualPanel.updateData(robot.getDataPlus().await(1000));
                    Thread.sleep(20);
                }catch (Exception e){
                    e.printStackTrace();
                }
            }
        }).start();

    }

    public void averageAngle(){
        new Thread(() -> {
            var sum = 0.0;
            int number = 500;
            for(int i = 0; i < number; i ++){
                try {
                    sum += robot.getEverything().await()[19-1];
                } catch (Exception ex) {
                    throw new RuntimeException(ex);
                }
            }
            System.out.println(sum/number);
        }).start();
    }


    @Override
    public void keyPressed(KeyEvent e) {
        int key = e.getKeyCode();
        float panStep = 20;

        switch (key) {
            case KeyEvent.VK_W -> visualPanel.offsetY += panStep;
            case KeyEvent.VK_S -> visualPanel.offsetY -= panStep;
            case KeyEvent.VK_A -> visualPanel.offsetX += panStep;
            case KeyEvent.VK_D -> visualPanel.offsetX -= panStep;
            case KeyEvent.VK_PLUS, KeyEvent.VK_EQUALS -> visualPanel.zoom *= 1.1f;
            case KeyEvent.VK_MINUS -> visualPanel.zoom /= 1.1f;
        }

        repaint();
    }

    @Override public void keyReleased(KeyEvent e) {}
    @Override public void keyTyped(KeyEvent e) {}

    @Override
    public void mouseWheelMoved(MouseWheelEvent e) {
        int notches = e.getWheelRotation();
        float factor = (notches > 0) ? 0.9f : 1.1f;
        visualPanel.zoom *= factor;
        repaint();
    }

    class VisualPanel extends JPanel{

        private float zoom = 20f; // pixels per unit
        private float offsetX = 0, offsetY = 0; // pan
        private Robot.DataPacketPlus dpp = new Robot.DataPacketPlus(0,0,0,0,0,0,0);

        public VisualPanel() {

            addMouseListener(new MouseAdapter() {
                @Override
                public void mouseClicked(MouseEvent e) {
                    Point screenPoint = e.getPoint();
                    float worldX = (screenPoint.x - getWidth() / 2f - offsetX) / zoom;
                    float worldY = -(screenPoint.y - getHeight() / 2f - offsetY) / zoom;
                    robot.sendTargetPos(worldX, worldY);
                }
            });
        }

        public void updateData(Robot.DataPacketPlus dpp) {
            this.dpp = dpp;
            if(dpp.distance()<2500){
                var x = dpp.x() + 0.0393701*dpp.distance()*Math.cos((dpp.yaw()+90)/180*Math.PI);
                var y = dpp.y() + 0.0393701*dpp.distance()*Math.sin((dpp.yaw()+90)/180*Math.PI);
                synchronized (maps){
                    maps.add(new Pos(x, y));
                }
            }
            repaint();
        }

        @Override
        protected void paintComponent(Graphics g) {
            super.paintComponent(g);
            drawScene((Graphics2D) g);
        }

        private void drawScene(Graphics2D g) {
            g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);

            int width = getWidth();
            int height = getHeight();


            // Flip back to draw text
            g.scale(1, 1);

            g.setColor(Color.WHITE);
            g.fillRect(0, 0, width, height);

            g.translate((double) width / 2 + offsetX, (double) height / 2 + offsetY);

            g.setColor(Color.BLACK);
            g.drawString(String.format("Position: (%.2f, %.2f)", dpp.x(), dpp.y()), -getWidth() / 2.0f + 10 - offsetX, -getHeight() / 2.0f + 20 - offsetY);
            g.drawString(String.format("Yaw: %.2f°", dpp.yaw()), -getWidth() / 2.0f + 10 - offsetX, -getHeight() / 2.0f + 40 - offsetY);
            g.drawString(String.format("Target Yaw: %.2f°", dpp.t_yaw()), -getWidth() / 2.0f - offsetX + 10, -getHeight() / 2.0f + 60 - offsetY);
            g.drawString(String.format("Zoom: %.1f", zoom), -getWidth() / 2.0f - offsetX + 10, -getHeight() / 2.0f + 80 - offsetY);

            g.scale(zoom, -zoom); // Y-up

            int px = Math.round(dpp.x());
            int py = Math.round(dpp.y());

            int pt_x = Math.round(dpp.t_x());
            int pt_y = Math.round(dpp.t_y());

            // Target
            g.setColor(Color.GRAY);
            g.fillOval(pt_x - 3, pt_y - 3, 6, 6);

            // Position dot
            g.setColor(Color.BLUE);
            g.fillOval(px - 5, py - 5, 10, 10);

            g.setColor(Color.ORANGE);

            ArrayList<Pos> cln;
            synchronized (maps) {
                cln = new ArrayList<>(maps);
            }
            for (var pos : cln) {
                g.fillOval((int) (pos.x - 1), (int) (pos.y - 2), 2, 2);
            }

            // Yaw direction
            {
                float yawRad = (float) Math.toRadians(dpp.yaw());
                int dx = (int) (30 * Math.cos(yawRad));
                int dy = (int) (30 * Math.sin(yawRad));
                g.setColor(Color.RED);
                g.drawLine(px, py, px + dx, py + dy);
            }

            // Target Yaw direction
            {
                float rad = (float) Math.toRadians(dpp.t_yaw());
                int dx = (int) (30 * Math.cos(rad));
                int dy = (int) (30 * Math.sin(rad));
                g.setColor(Color.GREEN);
                g.drawLine(px, py, px + dx, py + dy);
            }

            // Line to origin
            g.setColor(Color.GREEN.darker());
            g.drawLine(pt_x, pt_y, px, py);
        }
    }
}

