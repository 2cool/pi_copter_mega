package cc.dewdrop.ffplayer;


/**
 * Created by igor on 11/28/2017.
 */

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.location.Location;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;


public class GPSservice extends Service
{

    static public Location mLastLocation;
    private static final String TAG = "BOOMBOOMTESTGPS";
    private LocationManager mLocationManager = null;
    private static final int LOCATION_INTERVAL = 1000;
    private static final float LOCATION_DISTANCE = 10f;

    private class LocationListener implements android.location.LocationListener{
        ;
        public LocationListener(String provider)
        {
            // Log.e(TAG, "LocationListener " + provider);
            mLastLocation = new Location(provider);
        }
        @Override
        public void onLocationChanged(Location location)
        {
            // Log.e(TAG, "onLocationChanged: " + location);
            mLastLocation.set(location);
        }
        @Override
        public void onProviderDisabled(String provider)
        {
            // Log.e(TAG, "onProviderDisabled: " + provider);
        }
        @Override
        public void onProviderEnabled(String provider)
        {
            //Log.e(TAG, "onProviderEnabled: " + provider);
        }
        @Override
        public void onStatusChanged(String provider, int status, Bundle extras)
        {
            // Log.e(TAG, "onStatusChanged: " + provider);
        }
    }
    LocationListener mLocationListener = new LocationListener(LocationManager.GPS_PROVIDER);
    // LocationListener mLocationListener = new LocationListener(LocationManager.NETWORK_PROVIDER);

    @Override
    public IBinder onBind(Intent arg0)
    {
        return null;
    }
    @Override
    public int onStartCommand(Intent intent, int flags, int startId)
    {
        // Log.e(TAG, "onStartCommand");
        super.onStartCommand(intent, flags, startId);
        return START_STICKY;
    }
    @Override
    public void onCreate()
    {
        Log.e(TAG, "onCreate");
        initializeLocationManager();


/*      //NETWORK_PROVIDER
        try {
            mLocationManager.requestLocationUpdates(
                    LocationManager.NETWORK_PROVIDER, LOCATION_INTERVAL, LOCATION_DISTANCE,
                    mLocationListener);
        } catch (java.lang.SecurityException ex) {
            Log.i(TAG, "fail to request location update, ignore", ex);
        } catch (IllegalArgumentException ex) {
            Log.d(TAG, "network provider does not exist, " + ex.getMessage());
        }
        */
        try {
            mLocationManager.requestLocationUpdates(
                    LocationManager.GPS_PROVIDER, LOCATION_INTERVAL, LOCATION_DISTANCE,
                    mLocationListener);
        } catch (java.lang.SecurityException ex) {
            // Log.i(TAG, "fail to request location update, ignore", ex);
        } catch (IllegalArgumentException ex) {
            // Log.d(TAG, "gps provider does not exist " + ex.getMessage());
        }

    }
    @Override
    public void onDestroy()
    {
        Log.e(TAG, "onDestroy");
        super.onDestroy();
        if (mLocationManager != null) {

            try {
                mLocationManager.removeUpdates(mLocationListener);
            } catch (Exception ex) {
                // Log.i(TAG, "fail to remove location listners, ignore", ex);
            }

        }
    }
    private void initializeLocationManager() {
        //Log.e(TAG, "initializeLocationManager");
        if (mLocationManager == null) {
            mLocationManager = (LocationManager) getApplicationContext().getSystemService(Context.LOCATION_SERVICE);
        }
    }
}