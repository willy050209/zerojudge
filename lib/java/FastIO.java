package lib.java;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

/**
 * ZeroJudge Common I/O Library for Java 11 (OpenJDK 11.0.31)
 * Provides high-efficiency byte-stream reading and buffered writing.
 * Note: Declared package-private so it can easily bundle into a single submission file.
 */
class FastScanner {
    private final InputStream in;
    private final byte[] buffer = new byte[65536];
    private int head = 0;
    private int tail = 0;
    private boolean eof = false;

    public FastScanner() {
        this(System.in);
    }

    public FastScanner(InputStream in) {
        this.in = in;
    }

    private int nextByte() {
        if (head >= tail) {
            head = 0;
            try {
                tail = in.read(buffer);
            } catch (IOException e) {
                tail = -1;
            }
            if (tail <= 0) {
                eof = true;
                return -1;
            }
        }
        return buffer[head++] & 0xff;
    }

    public boolean hasNext() {
        int c = nextByte();
        while (c != -1 && c <= ' ') {
            c = nextByte();
        }
        if (c == -1) return false;
        head--; // push back the non-whitespace character
        return true;
    }

    public String next() {
        int c = nextByte();
        while (c != -1 && c <= ' ') {
            c = nextByte();
        }
        if (c == -1) return null;
        StringBuilder sb = new StringBuilder();
        while (c > ' ') {
            sb.append((char) c);
            c = nextByte();
        }
        return sb.toString();
    }

    public int nextInt() {
        int c = nextByte();
        while (c != -1 && c <= ' ') {
            c = nextByte();
        }
        if (c == -1) throw new IllegalStateException("EOF reached");
        boolean neg = false;
        if (c == '-') {
            neg = true;
            c = nextByte();
        } else if (c == '+') {
            c = nextByte();
        }
        int val = 0;
        while (c >= '0' && c <= '9') {
            val = val * 10 + (c - '0');
            c = nextByte();
        }
        return neg ? -val : val;
    }

    public long nextLong() {
        int c = nextByte();
        while (c != -1 && c <= ' ') {
            c = nextByte();
        }
        if (c == -1) throw new IllegalStateException("EOF reached");
        boolean neg = false;
        if (c == '-') {
            neg = true;
            c = nextByte();
        } else if (c == '+') {
            c = nextByte();
        }
        long val = 0;
        while (c >= '0' && c <= '9') {
            val = val * 10 + (c - '0');
            c = nextByte();
        }
        return neg ? -val : val;
    }

    public String nextLine() {
        int c = nextByte();
        if (c == -1) return null;
        StringBuilder sb = new StringBuilder();
        while (c != -1 && c != '\n' && c != '\r') {
            sb.append((char) c);
            c = nextByte();
        }
        if (c == '\r') {
            int next = nextByte();
            if (next != '\n' && next != -1) {
                head--;
            }
        }
        return sb.toString();
    }

    public boolean isEof() {
        return eof;
    }
}

class FastPrinter implements AutoCloseable {
    private final BufferedWriter writer;

    public FastPrinter() {
        this(System.out);
    }

    public FastPrinter(OutputStream out) {
        this.writer = new BufferedWriter(new OutputStreamWriter(out, StandardCharsets.UTF_8), 65536);
    }

    public void print(Object obj) throws IOException {
        writer.write(String.valueOf(obj));
    }

    public void println(Object obj) throws IOException {
        writer.write(String.valueOf(obj));
        writer.newLine();
    }

    public void println() throws IOException {
        writer.newLine();
    }

    public void flush() throws IOException {
        writer.flush();
    }

    @Override
    public void close() throws IOException {
        flush();
    }
}
