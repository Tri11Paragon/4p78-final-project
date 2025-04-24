import java.util.concurrent.TimeoutException;
import java.util.function.Consumer;

public abstract class Future<T, E extends Throwable> {
    private T val;
    private E err;
    private Consumer<T> then = ignore -> {
        this.accepted = false;
        this.notifyAll();
    };
    private Consumer<E> error = ignore -> {
        this.accepted = false;
        this.notifyAll();
    };
    private boolean accepted;

    protected synchronized void resolve(T val) {
        this.val = val;
        accepted = true;
        then.accept(this.val);
    }

    protected synchronized void resolve(E err) {
        this.err = err;
        accepted = true;
        error.accept(this.err);
    }

    public synchronized Future<T, E> then(Consumer<T> then) {
        this.then = then;
        if (this.val != null && !accepted) {
            accepted = true;
            this.then.accept(this.val);
        }
        return this;
    }

    public synchronized Future<T, E> error(Consumer<E> error) {
        this.error = error;
        if (this.err != null && !accepted) {
            accepted = true;
            this.error.accept(this.err);
        }
        return this;
    }

    public synchronized T await(long ms) throws InterruptedException, TimeoutException, E {
        if (this.val == null && this.err == null) this.wait(ms);
        if (this.val == null && this.err == null) throw new TimeoutException();
        if (accepted) throw new IllegalStateException("Value already accepted");
        accepted = true;
        if (this.err != null) throw this.err;
        return val;
    }

    public T await() throws InterruptedException, E, TimeoutException {
        return await(0);
    }
}
