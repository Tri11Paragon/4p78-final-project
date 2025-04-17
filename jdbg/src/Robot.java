import java.io.IOException;
import java.net.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.rmi.server.ExportException;
import java.util.function.Consumer;
import java.util.function.Function;

public class Robot{
    DatagramSocket socket = new DatagramSocket();

    final String ip;
    final int port;
    final InetAddress address;
    private final ByteBuffer buf = ByteBuffer.allocate(4096).order(ByteOrder.LITTLE_ENDIAN);


    public Robot() throws SocketException, UnknownHostException {
        this("192.168.5.11", 42069);
    }

    public Robot(String ip, int port) throws SocketException, UnknownHostException {
        this.ip = ip;
        address = InetAddress.getByName(ip);
        this.port = port;
    }

    public synchronized <T> T packet(int id, Consumer<ByteBuffer> setup, Function<ByteBuffer, T> func) throws IOException {
        setup.accept(buf.clear().limit(buf.capacity()).putInt(id));
        DatagramPacket packet = new DatagramPacket(buf.array(), buf.position(), address, port);
        socket.send(packet);

        buf.clear();
        DatagramPacket receivePacket = new DatagramPacket(buf.array(), buf.capacity());
        socket.setSoTimeout(1000);
        socket.receive(receivePacket);
        buf.limit(receivePacket.getLength());

        if(buf.getInt() != id)throw new IOException("ID miss match");
        var seq = buf.getInt();
        return func.apply(buf);
    }


    public void sendZero() throws IOException{
        packet(0, bb -> {}, bb -> null);
    }

    public record DataPacket(float yaw, float distance, float x, float y){}

    public DataPacket getData() throws IOException {
        return packet(1,  bb -> {}, bb -> {
            float yaw = bb.getFloat();
            float distance = bb.getFloat();
            float posX = bb.getFloat();
            float posY = bb.getFloat();

            return new DataPacket(yaw, distance, posX, posY);
        });
    }

    public void sendTargetPos(float x, float y) throws IOException{
        packet(2, bb -> bb.putFloat(x).putFloat(y), bb -> null);
    }

    public float[] getEverything() throws IOException{
        return packet(3,  bb -> {}, bb -> {
            float[] arr = new float[bb.remaining()/4];
            for(int i = 0; i < arr.length; i ++)
                arr[i] = bb.getFloat();
            return arr;
        });
    }


    public record DataPacketPlus(float yaw, float t_yaw, float distance, float x, float y, float t_x, float t_y){}

    public DataPacketPlus getDataPlus() throws IOException {
        return packet(4,
                bb -> {},
                bb -> {
                            float yaw = bb.getFloat();
                            float t_yaw = bb.getFloat();
                            float distance = bb.getFloat();
                            float x = bb.getFloat();
                            float y = bb.getFloat();
                            float t_x = bb.getFloat();
                            float t_y = bb.getFloat();

                            return new DataPacketPlus(yaw, t_yaw, distance, x, y, t_x, t_y);
        });
    }

    public record PID(float kp, float ki, float kd, int direction){}
    public PID getPID(int index) throws IOException{
        return packet(5,
                bb -> bb.putInt(index),
                bb -> new PID(bb.getFloat(), bb.getFloat(), bb.getFloat(), bb.getInt())
        );
    }

    public void setPID(int index, float kp, float ki, float kd, int direction) throws IOException{
        packet(6,
        bb -> bb
                    .putInt(index)
                    .putFloat(kp)
                    .putFloat(ki)
                    .putFloat(kd)
                    .putInt(direction),
        bb -> null
        );
    }
}
