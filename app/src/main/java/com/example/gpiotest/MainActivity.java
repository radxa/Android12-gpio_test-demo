package com.example.gpiotest;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.example.gpiotest.databinding.ActivityMainBinding;

import java.io.DataOutputStream;
import java.io.IOException;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";

    static {
        System.loadLibrary("gpiotest");
        System.loadLibrary("invokegpio");
    }

    private ActivityMainBinding binding;

    protected int signGPIOVal = -1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        Button invokeGPIO = binding.invokeGpio;
        invokeGPIO.setOnClickListener(new View.OnClickListener() {
            private void run() {

                /*Control gpio through cmd*/
//                String gpio_number = "138"; /* this gpio is an example gpio pin */
//                String level = "0"; /* 0 is low level, 1 is high level */
//                String exportPath = null;
//                String directionPath = null;
//                String valuePath = null;
//                Process process = null;
//                DataOutputStream dos = null;
//
//
//                exportPath = "echo " + gpio_number + " > /sys/class/gpio/export";
//                directionPath = "echo out > " + " /sys/class/gpio/gpio" + gpio_number + "/direction";
//                valuePath = "echo " + level + " > /sys/class/gpio/gpio" + gpio_number + "/value";
//
//                try {
//                    process = Runtime.getRuntime().exec("su");
//                    dos = new DataOutputStream(process.getOutputStream());
//                    dos.writeBytes(exportPath + "\n");
//                    dos.flush();
//                    dos.writeBytes(directionPath + "\n");
//                    dos.flush();
//                    dos.writeBytes(valuePath + "\n");
//                    dos.flush();
//                    dos.close();
//                } catch (IOException e) {
//                    throw new RuntimeException(e);
//                }

                Log.d(TAG, "onClick() start signGPIOVal = " + signGPIOVal);

                signGPIOVal = invokeGPIO();

            }

            @Override
            public void onClick(View view) {

                new Thread(this::run).start();

            }
        });
    }

    public native int invokeGPIO();
}
