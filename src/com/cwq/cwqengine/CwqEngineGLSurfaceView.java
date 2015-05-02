package com.cwq.cwqengine;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.MotionEvent;

public class CwqEngineGLSurfaceView extends GLSurfaceView {

    private final static String TAG = CwqEngineGLSurfaceView.class.getSimpleName();
    
    private CwqEngineRenderer mCwqEngineRenderer;
    
    public CwqEngineGLSurfaceView(Context context) {
        super(context);
        this.initView();
    }
    
    public CwqEngineGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        this.initView();
    }

    private void initView() {
        setEGLContextClientVersion(2);
        setFocusableInTouchMode(true);
        
        preserveEGLContextOnPause(true);
//        getHolder().setFormat(PixelFormat.TRANSLUCENT);
//        setEGLConfigChooser(8, 8, 8, 8, 16, 0);
    }
    
    /**
     * Control whether the EGL context is preserved when the GLSurfaceView is paused and
     * resumed.
     * <p>
     * If set to true, then the EGL context may be preserved when the GLSurfaceView is paused.
     * Whether the EGL context is actually preserved or not depends upon whether the
     * Android device that the program is running on can support an arbitrary number of EGL
     * contexts or not. Devices that can only support a limited number of EGL contexts must
     * release the  EGL context in order to allow multiple applications to share the GPU.
     * <p>
     * If set to false, the EGL context will be released when the GLSurfaceView is paused,
     * and recreated when the GLSurfaceView is resumed.
     * <p>
     *
     * The default is false.
     *
     * @param preserveOnPause preserve the EGL context when paused
     */
    public void preserveEGLContextOnPause(boolean preserveOnPause) {
        int sdkVersion = android.os.Build.VERSION.SDK_INT;
        if (sdkVersion >= 11) {
            try {
                this.getClass().getMethod("setPreserveEGLContextOnPause", boolean.class).invoke(this, preserveOnPause);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
    
    public void setCwqEngineRenderer(CwqEngineRenderer renderer) {
        mCwqEngineRenderer = renderer;
        setRenderer(renderer);
    }
    
    @Override
    public void onResume() {
        super.onResume();
        this.setRenderMode(RENDERMODE_CONTINUOUSLY);
        queueEvent(new Runnable() {
            @Override
            public void run() {
                mCwqEngineRenderer.handleOnResume();
            }
        });
    }
    
    @Override
    public void onPause() {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                mCwqEngineRenderer.handleOnPause();
            }
        });
        this.setRenderMode(RENDERMODE_WHEN_DIRTY);
        super.onPause();
    }
    
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        final int pointerNumber = event.getPointerCount();
        final int[] ids = new int[pointerNumber];
        final float[] xs = new float[pointerNumber];
        final float[] ys = new float[pointerNumber];

        for (int i = 0; i < pointerNumber; i++) {
            ids[i] = event.getPointerId(i);
            xs[i] = event.getX(i);
            ys[i] = event.getY(i);
        }

        switch (event.getAction() & MotionEvent.ACTION_MASK) {
            case MotionEvent.ACTION_POINTER_DOWN:
                final int indexPointerDown = event.getAction() >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
                final int idPointerDown = event.getPointerId(indexPointerDown);
                final float xPointerDown = event.getX(indexPointerDown);
                final float yPointerDown = event.getY(indexPointerDown);

                this.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        mCwqEngineRenderer.handleActionDown(idPointerDown, xPointerDown, yPointerDown);
                    }
                });
                break;

            case MotionEvent.ACTION_DOWN:
                // there are only one finger on the screen
                final int idDown = event.getPointerId(0);
                final float xDown = xs[0];
                final float yDown = ys[0];

                this.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        mCwqEngineRenderer.handleActionDown(idDown, xDown, yDown);
                    }
                });
                break;

            case MotionEvent.ACTION_MOVE:
                this.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        mCwqEngineRenderer.handleActionMove(ids, xs, ys, pointerNumber);
                    }
                });
                break;

            case MotionEvent.ACTION_POINTER_UP:
                final int indexPointUp = event.getAction() >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
                final int idPointerUp = event.getPointerId(indexPointUp);
                final float xPointerUp = event.getX(indexPointUp);
                final float yPointerUp = event.getY(indexPointUp);

                this.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        mCwqEngineRenderer.handleActionUp(idPointerUp, xPointerUp, yPointerUp);
                    }
                });
                break;

            case MotionEvent.ACTION_UP:
                // there are only one finger on the screen
                final int idUp = event.getPointerId(0);
                final float xUp = xs[0];
                final float yUp = ys[0];

                this.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        mCwqEngineRenderer.handleActionUp(idUp, xUp, yUp);
                    }
                });
                break;

            case MotionEvent.ACTION_CANCEL:
                this.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        mCwqEngineRenderer.handleActionCancel(ids, xs, ys, pointerNumber);
                    }
                });
                break;
        }

        return true;
    }
    
    @Override
    public boolean onKeyDown(final int keyCode, KeyEvent event) {
//        queueEvent(new Runnable() {
//            @Override
//            public void run() {
//                mCwqEngineRenderer.handleKeyDown(keyCode);
//            }
//        });
//        return true;
        return super.onKeyDown(keyCode, event);
    }
    
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK && event.isTracking()
                && !event.isCanceled()) {
            queueEvent(new Runnable() {
                @Override
                public void run() {
                    mCwqEngineRenderer.handleOnExit();
                }
            });
        }
        return super.onKeyUp(keyCode, event);
    }
}
