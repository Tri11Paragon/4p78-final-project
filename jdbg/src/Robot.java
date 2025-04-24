import java.io.IOException;
import java.net.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.HashMap;
import java.util.function.Consumer;
import java.util.function.Function;

public class Robot{
    DatagramSocket socket = new DatagramSocket();

    private int sequence = 0;
    final String ip;
    final int port;
    final InetAddress address;
    private final ByteBuffer outBuf = ByteBuffer.allocate(4096).order(ByteOrder.LITTLE_ENDIAN);

    private final HashMap<Integer, PFuture<?>> seqMap = new HashMap<>();

    public static abstract class PFuture<T> extends Future<T, Exception>{
        protected abstract void resolve(ByteBuffer bb);
    }


    public Robot() throws SocketException, UnknownHostException {
        this("192.168.4.1", 42069);
    }

    public Robot(String ip, int port) throws SocketException, UnknownHostException {
        this.ip = ip;
        address = InetAddress.getByName(ip);
        this.port = port;

        new Thread(() -> {
            var inBuf = ByteBuffer.allocate(4096).order(ByteOrder.LITTLE_ENDIAN);
            while(true){
                try{
                    inBuf.clear();
                    DatagramPacket receivePacket = new DatagramPacket(inBuf.array(), inBuf.capacity());
                    socket.receive(receivePacket);
                    inBuf.limit(receivePacket.getLength());

                    int squ = inBuf.getInt();
                    var future = seqMap.remove(squ);
                    if(future!=null) future.resolve(inBuf);
                }catch (Exception ignore){}
            }
        }).start();
    }

    public synchronized <T> PFuture<T> packet(int id, Consumer<ByteBuffer> setup, Function<ByteBuffer, T> func) {
        var f = new PFuture<T>(){
            @Override
            protected synchronized void resolve(ByteBuffer bb) {
                try{
                    if(id!=bb.getInt())
                        super.resolve(new Exception("ID Miss Match"));
                    else
                        super.resolve(func.apply(bb));
                }catch (Exception e){
                    super.resolve(e);
                }
            }
        };
        seqMap.put(sequence, f);
        try{
            setup.accept(outBuf.clear().limit(outBuf.capacity()).putInt(sequence++).putInt(id));
            DatagramPacket packet = new DatagramPacket(outBuf.array(), outBuf.position(), address, port);
            socket.send(packet);
        }catch (Exception e){
            f.resolve(e);
        }

        return f;
    }


    public PFuture<Void> sendZero() {
        return packet(0, bb -> {}, bb -> null);
    }

    public record DataPacket(float yaw, float distance, float x, float y){}

    public PFuture<DataPacket> getData()  {
        return packet(1,  bb -> {}, bb -> {
            float yaw = bb.getFloat();
            float distance = bb.getFloat();
            float posX = bb.getFloat();
            float posY = bb.getFloat();

            return new DataPacket(yaw, distance, posX, posY);
        });
    }

    public PFuture<Void> sendTargetPos(float x, float y) {
        return packet(2, bb -> bb.putFloat(x).putFloat(y), bb -> null);
    }

    public PFuture<float[]> getEverything() {
        return packet(3,  bb -> {}, bb -> {
            float[] arr = new float[bb.remaining()/4];
            for(int i = 0; i < arr.length; i ++)
                arr[i] = bb.getFloat();
            return arr;
        });
    }


    public record DataPacketPlus(float yaw, float t_yaw, float distance, float x, float y, float t_x, float t_y){}

    public PFuture<DataPacketPlus> getDataPlus() {
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
    public PFuture<PID> getPID(int index) {
        return packet(5,
                bb -> bb.putInt(index),
                bb -> new PID(bb.getFloat(), bb.getFloat(), bb.getFloat(), bb.getInt())
        );
    }

    public PFuture<Void> setPID(int index, float kp, float ki, float kd, int direction) {
        return packet(6,
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
