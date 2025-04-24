import javax.swing.*;
import java.awt.*;

public class PidPanel extends JPanel {

    public final double SCALE = 10000.0;

    public PidPanel(Robot robot){
        String[] pidNames = {"angle", "pos", "turn"};
        JComboBox<String> indexDropdown = new JComboBox<>(pidNames);

        JSlider kpSlider = new JSlider(0, (int)(SCALE*10), 0);
        JSlider kiSlider = new JSlider(0, (int)(SCALE*150), 0);
        JSlider kdSlider = new JSlider(0, (int)(SCALE*2), 0);

        JLabel kpValue = new JLabel("0.00");
        JLabel kiValue = new JLabel("0.00");
        JLabel kdValue = new JLabel("0.00");

        kpSlider.addChangeListener(e -> kpValue.setText(String.format("%.5f", kpSlider.getValue() / SCALE)));
        kiSlider.addChangeListener(e -> kiValue.setText(String.format("%.5f", kiSlider.getValue() / SCALE)));
        kdSlider.addChangeListener(e -> kdValue.setText(String.format("%.5f", kdSlider.getValue() / SCALE)));

        JRadioButton directButton = new JRadioButton("Direct", true);
        JRadioButton reverseButton = new JRadioButton("Reverse");
        ButtonGroup dirGroup = new ButtonGroup();
        dirGroup.add(directButton);
        dirGroup.add(reverseButton);

        JButton getPidButton = new JButton("Get PID");
        JButton setPidButton = new JButton("Set PID");

        this.setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
        this.setBorder(BorderFactory.createTitledBorder("PID Settings"));
        this.setAlignmentX(Component.LEFT_ALIGNMENT);
        this.setPreferredSize(new Dimension(600, 250));

        // PID index selector
        JPanel indexRow = new JPanel(new FlowLayout(FlowLayout.LEFT));
        indexRow.add(new JLabel("Index:"));
        indexRow.add(indexDropdown);
        this.add(indexRow);

        // Kp row
        JPanel kpRow = new JPanel(new BorderLayout());
        kpRow.add(new JLabel("Kp:"), BorderLayout.WEST);
        kpRow.add(kpSlider, BorderLayout.CENTER);
        kpRow.add(kpValue, BorderLayout.EAST);
        kpRow.setBorder(BorderFactory.createEmptyBorder(2, 10, 2, 10));
        this.add(kpRow);

        // Ki row
        JPanel kiRow = new JPanel(new BorderLayout());
        kiRow.add(new JLabel("Ki:"), BorderLayout.WEST);
        kiRow.add(kiSlider, BorderLayout.CENTER);
        kiRow.add(kiValue, BorderLayout.EAST);
        kiRow.setBorder(BorderFactory.createEmptyBorder(2, 10, 2, 10));
        this.add(kiRow);

        // Kd row
        JPanel kdRow = new JPanel(new BorderLayout());
        kdRow.add(new JLabel("Kd:"), BorderLayout.WEST);
        kdRow.add(kdSlider, BorderLayout.CENTER);
        kdRow.add(kdValue, BorderLayout.EAST);
        kdRow.setBorder(BorderFactory.createEmptyBorder(2, 10, 2, 10));
        this.add(kdRow);

        // Direction row
        JPanel dirRow = new JPanel(new FlowLayout(FlowLayout.LEFT));
        dirRow.add(new JLabel("Direction:"));
        dirRow.add(directButton);
        dirRow.add(reverseButton);
        this.add(Box.createVerticalStrut(5));
        this.add(dirRow);

        // Buttons row
        JPanel buttonRow = new JPanel(new FlowLayout(FlowLayout.LEFT));
        buttonRow.add(getPidButton);
        buttonRow.add(setPidButton);
        this.add(Box.createVerticalStrut(5));
        this.add(buttonRow);

        getPidButton.addActionListener(e -> {
            int index = indexDropdown.getSelectedIndex();
            robot.getPID(index).then(pid -> {
                kpSlider.setValue((int) (pid.kp() * SCALE));
                kiSlider.setValue((int) (pid.ki() * SCALE));
                kdSlider.setValue((int) (pid.kd() * SCALE));
                if (pid.direction() == 0) {
                    directButton.setSelected(true);
                } else {
                    reverseButton.setSelected(true);
                }
            }).error(ex -> {
                ex.printStackTrace();
                JOptionPane.showMessageDialog(this, "Failed to get PID: " + ex.getMessage());
            });
        });

        setPidButton.addActionListener(e -> {
            int index = indexDropdown.getSelectedIndex();
            float kp = (float) (kpSlider.getValue() / SCALE);
            float ki = (float) (kiSlider.getValue() / SCALE);
            float kd = (float) (kdSlider.getValue() / SCALE);
            int direction = directButton.isSelected() ? 0 : 1;

            robot.setPID(index, kp, ki, kd, direction).error(ex -> {
                ex.printStackTrace();
                JOptionPane.showMessageDialog(this, "Failed to set PID: " + ex.getMessage());
            });
        });
        getPidButton.doClick();
        indexDropdown.addActionListener(a -> {
            getPidButton.doClick();
        });
    }
}
