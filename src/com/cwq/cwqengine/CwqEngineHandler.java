package com.cwq.cwqengine;

import java.lang.ref.WeakReference;

import android.os.Handler;
import android.os.Message;

public class CwqEngineHandler extends Handler {
    protected WeakReference<CwqEngineActivity> mWeakEngine;
    protected static final int ENGINE_ERROR = -1000;
    
    public CwqEngineHandler(CwqEngineActivity engine) {
        super();
        mWeakEngine = new WeakReference<CwqEngineActivity>(engine);
    }
    
    /**
     * if engine is unavailable, msg.what = ENGINE_ERROR
     */
    @Override
    public void handleMessage(Message msg) {
        CwqEngineActivity engine = mWeakEngine.get();
        if (engine == null || !engine.isEngineOK()) {
            msg.what = ENGINE_ERROR;
            return;
        }
        
        switch (msg.what) {

        default:
            break;
        }
    }
    
    
    /**
     * call by native to post message
     * @param weakThiz
     * @param what
     * @param arg1
     * @param arg2
     */
    private static void postEventFromNative(Object weakEngine, int what, 
            int arg1, int arg2, Object obj) {
        if (weakEngine == null)
            return;
        
        CwqEngineActivity engine = ((WeakReference<CwqEngineActivity>)weakEngine).get();
        if (engine == null && !engine.isEngineOK()) {
            return;
        }
        
        if (engine.mHandler != null) {
            Message m = engine.mHandler.obtainMessage(what, arg1, arg2, obj);
            engine.mHandler.sendMessage(m);
        }
    }
}
