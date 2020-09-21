package me.doubleye.mortydemo

import android.Manifest
import android.R.attr.password
import android.R.attr.path
import android.app.NativeActivity
import android.os.Bundle
import android.os.Environment
import android.util.Log
import androidx.core.app.ActivityCompat
import java.io.FileOutputStream


class MainActivity : NativeActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        ActivityCompat.requestPermissions(
            this, arrayOf(
                Manifest.permission.READ_EXTERNAL_STORAGE,
                Manifest.permission.WRITE_EXTERNAL_STORAGE
            ), 1
        )

    }
}
