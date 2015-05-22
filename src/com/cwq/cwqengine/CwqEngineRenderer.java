package com.cwq.cwqengine;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView;

class CwqEngineRenderer implements GLSurfaceView.Renderer {
    
    private final static String TAG = CwqEngineRenderer.class.getSimpleName();

    private final static long NANOSECONDSPERSECOND = 1000000000L;
    private final static long NANOSECONDSPERMICROSECOND = 1000000;
    
    private long sAnimationInterval = (long) (1.0 / 60 * NANOSECONDSPERSECOND);
    
    private long mLastTickInNanoSeconds;
    
    public void setFPS(int fps) {
        sAnimationInterval = (long) (1.0 / fps * NANOSECONDSPERSECOND);
    }
    
    private boolean isControlFPS = false;
    public void setControlFPS(boolean isControl) {
        isControlFPS = isControl;
    }
    
    private CwqEngine mCwqEngine;
    
    private int mScreenWidth;
    private int mScreenHeight;
    
    public CwqEngineRenderer(CwqEngine cwqEngine) {
        mCwqEngine = cwqEngine;
    }
    
    public void setScreenWidthAndHeight(final int surfaceWidth, final int surfaceHeight) {
        this.mScreenWidth = surfaceWidth;
        this.mScreenHeight = surfaceHeight;
    }
    
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        mCwqEngine.onSurfaceCreated(mScreenWidth, mScreenHeight);
        mLastTickInNanoSeconds = System.nanoTime();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        mCwqEngine.onSurfaceChanged(width, height);
    }
    
    @Override
    public void onDrawFrame(GL10 gl) {
        mCwqEngine.onDrawFrame();
        
        if (isControlFPS) {
            long nowInNanoSeconds = System.nanoTime();
            long interval = nowInNanoSeconds - mLastTickInNanoSeconds;
            if (interval < sAnimationInterval) {
                try {
                    Thread.sleep((sAnimationInterval - interval) / NANOSECONDSPERMICROSECOND);
                } catch (final Exception e) {
                    
                }
            }
        }
        mLastTickInNanoSeconds = System.nanoTime();
    }
    
    public void handleOnResume() {
        mCwqEngine.onResume();
    }
    
    public void handleOnPause() {
        mCwqEngine.onPause();
    }
    
    public void handleOnExit() {
        mCwqEngine.onExit();
    }
    
    public void handleKeyDown(int keyCode) {
        mCwqEngine.onKeyDown(keyCode);
    }
    
    public void handleActionDown(final int pID, final float pX, final float pY) {
        mCwqEngine.onTouchesBegin(pID, pX, pY);
    }

    public void handleActionUp(final int pID, final float pX, final float pY) {
        mCwqEngine.onTouchesEnd(pID, pX, pY);
    }

    public void handleActionCancel(final int[] pIDs, final float[] pXs, final float[] pYs, int pNum) {
        mCwqEngine.onTouchesCancel(pIDs, pXs, pYs, pNum);
    }

    public void handleActionMove(final int[] pIDs, final float[] pXs, final float[] pYs, int pNum) {
        mCwqEngine.onTouchesMove(pIDs, pXs, pYs, pNum);
    }
}
