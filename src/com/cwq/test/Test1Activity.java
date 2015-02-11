package com.cwq.test;

import android.content.Intent;
import android.os.Bundle;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

import com.cwq.cwqengine.CwqEngineActivity;
import com.cwq.cwqengine.CwqEngineHandler;
import com.cwq.cwqengine.R;

public class Test1Activity extends CwqEngineActivity {
    
    class TestHandler extends CwqEngineHandler {

        public TestHandler(CwqEngineActivity engine) {
            super(engine);
        }
        
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
            case 2:
                Toast.makeText(mWeakEngine.get(), "handle 2", Toast.LENGTH_SHORT).show();
                break;

            default:
                break;
            }
        }
    }
    
    private Button button1;
    private Button button2;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState, R.layout.main, R.id.glsurface);
        mHandler = new TestHandler(this);
        
        button1 = (Button) findViewById(R.id.button1);
        button1.setOnClickListener(new OnClickListener() {
            
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(Test1Activity.this, Test2Activity.class);
                startActivity(intent);
            }
        });
    }
}
