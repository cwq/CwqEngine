package com.cwq.cwqengine;

import java.lang.ref.WeakReference;

import android.app.Activity;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;

public class CwqEngineActivity extends Activity {
    
    private final static String TAG = CwqEngineActivity.class.getSimpleName();
    
    private CwqEngineGLSurfaceView mGlSurfaceView;
    
    protected CwqEngine mCwqEngine;
    
    protected CwqEngineHandler mHandler;
    
    private static volatile boolean mIsLibLoaded = false;
    private void onLoadNativeLibraries() {
        synchronized(CwqEngineActivity.class) {
            if (!mIsLibLoaded) {
                try {
                    System.loadLibrary("ffmpeg");
                    ApplicationInfo ai = getPackageManager().getApplicationInfo(getPackageName(), PackageManager.GET_META_DATA);
                    Bundle bundle = ai.metaData;
                    String libName = bundle.getString("android.app.lib_name");
                    System.loadLibrary(libName);
                    mIsLibLoaded = true;
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        this.init(false, 0, 0);
    }
    
    protected void onCreate(Bundle savedInstanceState, int layoutResID, int cwqGLSurfaceID) {
        super.onCreate(savedInstanceState);
        
        this.init(true, layoutResID, cwqGLSurfaceID);
    }
    
    private void init(boolean useLayout, int layoutResID, int cwqGLSurfaceID) {
        //load so
        onLoadNativeLibraries();
        
        //init engine
        mCwqEngine = new CwqEngine();
        mCwqEngine.setJavaWeakEngine(new WeakReference<CwqEngineActivity>(this));
        mCwqEngine.setAssetManager(getAssets());
        
        //new handle
        mHandler = new CwqEngineHandler(this);
        
        //init glsurfaceview
        if (useLayout) {
            setContentView(layoutResID);
            mGlSurfaceView = (CwqEngineGLSurfaceView) findViewById(cwqGLSurfaceID);
        } else {
            mGlSurfaceView = new CwqEngineGLSurfaceView(this);
            setContentView(mGlSurfaceView);
        }
        //set renderer
        mGlSurfaceView.setCwqEngineRenderer(new CwqEngineRenderer(mCwqEngine));
    }
    
    public boolean isEngineOK() {
        if (mCwqEngine != null && CwqEngine.getCPtr(mCwqEngine) != 0) {
            return true;
        }
        return false;
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
        mCwqEngine.delete();
    }
    
    @Override
    protected void onResume() {
        super.onResume();
        mGlSurfaceView.onResume();
    }
    
    @Override
    protected void onPause() {
        super.onPause();
        mGlSurfaceView.onPause();
    }
    
    public void runOnGLThread(final Runnable runnable) {
        mGlSurfaceView.queueEvent(runnable);
    }
}
