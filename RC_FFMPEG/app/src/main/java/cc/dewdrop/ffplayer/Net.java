package cc.dewdrop.ffplayer;

import android.content.Context;
import android.util.Log;

import java.io.InputStream;
import java.io.OutputStream;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.Socket;
import java.net.SocketException;
import java.util.Collections;
import java.util.Enumeration;
import java.util.List;




public class Net {
    //private static final int MAX_UDP_DATAGRAM_LEN = 1500;



    /**
     * Get IP address from first non-localhost interface
     * @param useIPv4   true=return ipv4, false=return ipv6
     * @return  address or empty string
     */
    public static String getIPAddress(boolean useIPv4) {
        try {
            List<NetworkInterface> interfaces = Collections.list(NetworkInterface.getNetworkInterfaces());
            for (NetworkInterface intf : interfaces) {
                List<InetAddress> addrs = Collections.list(intf.getInetAddresses());
                for (InetAddress addr : addrs) {
                    if (!addr.isLoopbackAddress()) {
                        String sAddr = addr.getHostAddress();
                        //boolean isIPv4 = InetAddressUtils.isIPv4Address(sAddr);
                        boolean isIPv4 = sAddr.indexOf(':')<0;

                        if (useIPv4) {
                            if (isIPv4)
                                return sAddr;
                        } else {
                            if (!isIPv4) {
                                int delim = sAddr.indexOf('%'); // drop ip6 zone suffix
                                return delim<0 ? sAddr.toUpperCase() : sAddr.substring(0, delim).toUpperCase();
                            }
                        }
                    }
                }
            }
        } catch (Exception ignored) { } // for now eat exceptions
        return "";
    }










    public static boolean net_runing=true;
    private static  int SERVER_PORT = 9876;
    static private boolean ip_OK=false;

    public static String getIpAddress() {
        try {
            for (Enumeration en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
                NetworkInterface intf = (NetworkInterface) en.nextElement();
                for (Enumeration enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();) {
                    InetAddress inetAddress = (InetAddress) enumIpAddr.nextElement();
                    if (!inetAddress.isLoopbackAddress()&&inetAddress instanceof Inet4Address) {
                        String ipAddress=inetAddress.getHostAddress().toString();
                        Log.e("IP address",""+ipAddress);
                        return ipAddress;
                    }
                }
            }
        } catch (SocketException ex) {
            Log.e("Socket exception", ex.toString());
        }
        return null;
    }



    public Net(int port,int timeOut){
        SERVER_PORT =port;
    }
    public void stop(){

        //udp_server_run=false;
        //client_runing=false;
    }

    static InetAddress copterAddress,myIP=null;
    int port=0;

    static int threads=0;

    static Context context;

    public void start(){
        Log.d("NET","start");
        if (threads>0 || net_runing==false) {
            Log.d("NET","exit");
            return;
        }
        threads++;
        Thread thread = new Thread() {
            @Override
            public void run() {
                //		Disk.close();
                //		Disk.createOrOpen();
                while (net_runing) {

                    //connect2esp();
                    String myIP=getIpAddress();
                //    String myIP=getIPAddress(true);


                   // String serverIPandPort[]={"192.168.1.112:9876","192.168.1.112:9876"};//Disk.getIP(myIP);
                    String serverIPandPort[]=Disk.getIP(myIP);



                    int i=0;
                    while (serverIPandPort[i]!=null && serverIPandPort[i].length()>10) {

                        Log.d("NET3","CONNECT TO "+Integer.toString(i));
                        if (runTCPClient(serverIPandPort[i])==true) {
                            if (ip_OK == false) {
                                i++;
                                if (i >= serverIPandPort.length)
                                    i = 0;
                            }
                        }

                    }
                }
            }
        };
        thread.start();
    }






    byte buffer[]=new byte[16384];

    final int IP=0;
    final int PORT=1;
    boolean dataloaded=false;
    public boolean runTCPClient(String ip_port){

        if (dataloaded==false) {
            dataloaded=true;
            Disk.loadLatLonAlt("/sdcard/RC/lostCon_location.save", true);
        }


        if (ip_port==null)
            return false;
        String s[]=ip_port.split(":");
        Socket socket=null;
        OutputStream out=null;
        InputStream in=null;
        boolean ret=true;
        try{
            in=null;
            out=null;
            try {

                copterAddress=InetAddress.getByName(s[IP]);
                SERVER_PORT =Integer.parseInt(s[PORT]);

                do {
                    try {
                        socket = new Socket(copterAddress, SERVER_PORT);
                        break;
                    } catch (java.net.SocketException e) {
                        socket.close();
                        Log.i("NET",e.getMessage());
                    }
                }while(true);


                socket.setSoTimeout(3000);
                out=socket.getOutputStream();
                in =socket.getInputStream();


                Log.i("NET","TCP_CLIENT STARTING...");


                Commander.new_connection();
                while(net_runing){

                    int len =Commander.get(buffer);
                    //Log.i("UDP",Integer.toString((cnt)));
                    out.write(buffer, 0, len);
                    out.flush();

                    //Log.i("UDP","READING... "+Integer.toString(cnt));

                    try{
                        len=in.read(buffer);
                        //	Log.i("UDP","<-"+buffer);
                        Commander.link=true;
                        ip_OK=true;
                        Telemetry.bufferReader_(buffer,len);


                    }catch(java.net.SocketTimeoutException e){
                        socket.setSoTimeout(5000);
                        Log.i("NET","SocketTimeoutException");
                        //if (++timeOutErrors>=1){
                        //Log.i("UDP","SocketTimeoutException & reboot");
                        if (socket.isClosed() == false && socket.isConnected()) {
                            socket.shutdownInput();
                            socket.shutdownOutput();
                            socket.close();

                            if (Commander.link)
                                Disk.saveLatLonAlt("/sdcard/RC/lostCon_location.save",Telemetry.lat,Telemetry.lon,Telemetry._alt);
                            Commander.link=false;

                        }

                        break;
                        //}
                    }

                }

            } catch (Exception e) {
                Log.i("NET", "EXCEPTION "+e.getMessage());
                if (e.getMessage().indexOf("ECONNREFUSED")!=-1)
                    ret= true;
                else
                    ret= true;

            }

            finally {
                if (Commander.link)
                    Disk.saveLatLonAlt("/sdcard/RC/lostCon_location.save",Telemetry.lat,Telemetry.lon,Telemetry._alt);
                Commander.link=false;
                if (MainActivity.drawView!=null)
                    MainActivity.drawView.postInvalidate();
                Log.i("NET","TCP_CLIENT KILLED!");


            }
        }catch (Exception e){
            Log.i("NET", "TCP CLIENT EXCEPTION "+e.getMessage());
        }
        return true;
    }
}
