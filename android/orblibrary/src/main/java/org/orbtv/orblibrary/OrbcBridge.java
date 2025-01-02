package org.orbtv.orblibrary;


import android.content.Context;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.provider.Settings;
import android.util.Log;

import org.orbtv.orbservice.IBridgeSession;
import org.orbtv.orbservice.IBrowserSession;

class OrbcBridge
{
    private static final String TAG = "OrbBridge";

    private static final String ORB_PACKAGE_NAME = "org.orbtv.orbservice";
    private static final String BRIDGE_CLASS_NAME = "org.orbtv.orbservice.BridgeService";

    private IBridgeSession mBridgeSession;
    private BrowserView mBrowserView;

    public OrbcBridge(Context context, BrowserView view)
    {
        ComponentName component = new ComponentName(ORB_PACKAGE_NAME, BRIDGE_CLASS_NAME);
        Intent intent = new Intent().setComponent(component);
        Log.d(TAG, "Try to resolve Orb Bridge service intent: " +
                context.getPackageManager().queryIntentServices(intent, 0));
        context.bindService(intent, mBridgeConnection, Context.BIND_AUTO_CREATE);
    }

    private final ServiceConnection mBridgeConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder binder) {
            Log.d(TAG, "Bridge service connected");
            mBridgeSession = IBridgeSession.Stub.asInterface(binder);
            try {
                mBridgeSession.initialise(new OrbcBrowserSession());
            } catch (RemoteException e) {
               Log.e(TAG, "IBridgeSession.initialise failed: " + e.getMessage());
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            Log.e(TAG, "ORB service disconnected");
        }
    };

    private static class OrbcBrowserSession extends IBrowserSession.Stub
    {
        OrbcBrowserSession()
        {
        }

        @Override
        public boolean dispatchKeyEvent(int action, int android_code, int tv_code)
        {
            return false;
        }

        @Override
        public void loadApplication(int app_id, byte[] url, int[] graphic_ids)
        {
        }

        @Override
        public void showApplication()
        {
        }

        @Override
        public void hideApplication()
        {
        }

        @Override
        public void dispatchEvent(byte[] type, byte[] properties)
        {
        }

        @Override
        public void dispatchTextInput(byte[] text)
        {
        }
    }

}
