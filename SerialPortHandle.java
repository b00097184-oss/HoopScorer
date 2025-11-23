import jssc.SerialPort;
import jssc.SerialPortException;

class SerialPortHandle {
    SerialPort sp;
    String path;

    public SerialPortHandle(String path) {
        this.sp = new SerialPort(path);
        this.path = path;
        try {
            sp.openPort();
            sp.setParams(9600, 8, 1, 0);
            while (sp.getInputBufferBytesCount() > 0) sp.readBytes();
        } catch (SerialPortException e) { e.printStackTrace(); }
    }

    public String readLine() {
        StringBuilder string = new StringBuilder();
        while (true) {
            try {
                byte[] buffer = sp.readBytes(1);
                if (buffer == null || buffer.length == 0) continue;
                char c = (char) buffer[0];
                if (c == '\n' || c == '\r') {
                    if (string.length() == 0) continue;
                    break;
                }
                string.append(c);
            } catch (SerialPortException e) { break; }
        }
        return string.toString().trim();
    }

    public void send(String msg) {
        try {
            sp.writeBytes((msg + "\n").getBytes());
        } catch (SerialPortException e) {
            e.printStackTrace();
        }
    }
}