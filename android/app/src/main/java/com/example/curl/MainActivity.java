package com.example.curl;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.widget.TextView;

import com.example.curl.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'curl' library on application startup.
    static {
        System.loadLibrary("http_client");
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        request();
    }

    /**
     * A native method that is implemented by the 'curl' native library,
     * which is packaged with this application.
     */
    public native void request();
}