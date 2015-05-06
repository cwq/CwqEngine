package com.cwq.test;

import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

import com.cwq.cwqengine.CwqEngineActivity;
import com.cwq.cwqengine.R;

public class Test2Activity extends CwqEngineActivity {

    private Button button1;
    private Button button2;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState, R.layout.main, R.id.glsurface);
        
        button1 = (Button) findViewById(R.id.button1);
        button1.setOnClickListener(new OnClickListener() {
            
            @Override
            public void onClick(View v) {
                postEventToEngine(false, 1, 1, 1, null);
            }
        });
        
        button2 = (Button) findViewById(R.id.button2);
        button2.setOnClickListener(new OnClickListener() {
            
            @Override
            public void onClick(View v) {
                runOnGLThread(new Runnable() {
                    @Override
                    public void run() {
                        postEventToEngine(false, 2, 2, 2, null);
                    }
                });
            }
        });
    }
    
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK && event.isTracking()
                && !event.isCanceled()) {
            onExit();
        }
        return super.onKeyUp(keyCode, event);
    }
}
