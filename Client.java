package cn.ikanke;

public class Client {
    static {
        System.loadLibrary("gxmpp");
    }
    Client() {
    	init();
    }
    protected void finalize() {
    	finit();
    }
    private native void init();
    private native void finit();
    public native void Login(String jid, String password);
    public native void Logout();
    public native int SendMessage(String to, String message);

}
