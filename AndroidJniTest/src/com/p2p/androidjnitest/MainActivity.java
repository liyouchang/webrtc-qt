package com.p2p.androidjnitest;

import com.video.play.TunnelCommunication;

import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;
import android.view.View;
import android.widget.Button;

public class MainActivity extends Activity {
	private Button mButtonInit = null; 
	private Button mButtonOpen = null; 
	private Button mButtonClose = null; 
	private Button mButtonVideoStart = null; 

	TunnelCommunication jni ;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		jni = new TunnelCommunication();

		mButtonInit = (Button)this.findViewById(R.id.buttonInit);
		mButtonOpen = (Button)this.findViewById(R.id.buttonOpen);
		mButtonClose = (Button)this.findViewById(R.id.buttonClose);
		mButtonVideoStart = (Button)this.findViewById(R.id.buttonVideoStart);

	         //按钮监听
		mButtonInit.setOnClickListener(new View.OnClickListener() 
	         {
	             
	             @Override
	             public void onClick(View v) 
	             {
	            	 jni.tunnelInitialize("");
	             }
	         });
	         
	         //按钮监听
		mButtonOpen.setOnClickListener(new View.OnClickListener() 
	         {
	             @Override
	             public void onClick(View v) 
	             {
	                 jni.openTunnel("123456");
	             }
	         });
		mButtonClose.setOnClickListener(new View.OnClickListener() 
        {
            @Override
            public void onClick(View v) 
            {
                jni.closeTunnel("123456");
            }
        });
		
		mButtonVideoStart.setOnClickListener(new View.OnClickListener() 
        {
            @Override
            public void onClick(View v) 
            {
                jni.askMediaData("123456");
            }
        });
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

}
