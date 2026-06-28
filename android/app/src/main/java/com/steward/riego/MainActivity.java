package com.steward.riego;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.IBinder;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.os.Handler;
import android.os.Looper;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

public class MainActivity extends AppCompatActivity {
    private static final int REQUEST_PERMISSIONS = 100;
    private static final int REQUEST_ENABLE_BT = 101;

    private TextView tvTemp;
    private TextView tvHum;
    private TextView tvPump;
    private TextView tvStatus;
    private Spinner spinnerDevices;
    private Button btnToggleConnection;
    private EditText etThreshold;
    private Button btnSetThreshold;
    private Button btnActivatePump;

    private BluetoothService bluetoothService;
    private boolean serviceBound = false;
    private boolean isConnected = false;
    private boolean isConnecting = false;
    private boolean shouldAutoConnect = true;
    private List<BluetoothDevice> pairedDevices = new ArrayList<>();
    private DatabaseHelper db;

    private Handler pumpHandler = new Handler(Looper.getMainLooper());
    private Runnable turnOffPumpRunnable;
    private Runnable cooldownRunnable;

    private static final long PUMP_ON_DURATION_MS = 8000;
    private static final long PUMP_COOLDOWN_MS = 2000;

    private final ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            BluetoothService.LocalBinder binder = (BluetoothService.LocalBinder) service;
            bluetoothService = binder.getService();
            serviceBound = true;
            bluetoothService.setListener(bluetoothListener);
            autoConnect();
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            bluetoothService = null;
            serviceBound = false;
        }
    };

    private final BluetoothService.BluetoothListener bluetoothListener = new BluetoothService.BluetoothListener() {
        @Override
        public void onDataReceived(float temp, float hum, boolean pumpOn) {
            tvTemp.setText(String.format("%.1f C", temp));
            tvHum.setText(String.format("%.1f %%", hum));
            tvPump.setText(pumpOn ? "ENCENDIDA" : "APAGADA");
            tvPump.setTextColor(getColor(pumpOn ? android.R.color.holo_green_dark : android.R.color.holo_red_dark));

            if (db != null) {
                SensorReading reading = new SensorReading(temp, hum, pumpOn, System.currentTimeMillis());
                db.insertReading(reading);
            }
        }

        @Override
        public void onConnectionStateChanged(boolean connected) {
            isConnected = connected;
            isConnecting = false;
            tvStatus.setText(connected ? "Conectado" : "Desconectado");
            tvStatus.setTextColor(getColor(connected ? android.R.color.holo_green_dark : android.R.color.holo_red_dark));
            btnToggleConnection.setText(connected ? "Desconectar" : "Conectar");
        }

        @Override
        public void onError(String error) {
            Toast.makeText(MainActivity.this, error, Toast.LENGTH_SHORT).show();
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        tvTemp = findViewById(R.id.tvTemp);
        tvHum = findViewById(R.id.tvHum);
        tvPump = findViewById(R.id.tvPump);
        tvStatus = findViewById(R.id.tvStatus);
        spinnerDevices = findViewById(R.id.spinnerDevices);
        btnToggleConnection = findViewById(R.id.btnToggleConnection);
        etThreshold = findViewById(R.id.etThreshold);
        btnSetThreshold = findViewById(R.id.btnSetThreshold);
        btnActivatePump = findViewById(R.id.btnActivatePump);

        btnToggleConnection.setOnClickListener(v -> toggleConnection());
        btnSetThreshold.setOnClickListener(v -> sendThreshold());
        btnActivatePump.setOnClickListener(v -> activatePump());

        Button btnHistory = findViewById(R.id.btnHistory);
        btnHistory.setOnClickListener(v -> {
            Intent intent = new Intent(MainActivity.this, HistoryActivity.class);
            startActivity(intent);
        });

        db = new DatabaseHelper(this);

        checkPermissions();
        bindService();
        promptEnableBluetooth();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        pumpHandler.removeCallbacksAndMessages(null);
        if (serviceBound) {
            unbindService(serviceConnection);
            serviceBound = false;
        }
    }

    private void checkPermissions() {
        List<String> permissions = new ArrayList<>();
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH) != PackageManager.PERMISSION_GRANTED) {
            permissions.add(Manifest.permission.BLUETOOTH);
        }
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_ADMIN) != PackageManager.PERMISSION_GRANTED) {
            permissions.add(Manifest.permission.BLUETOOTH_ADMIN);
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                permissions.add(Manifest.permission.BLUETOOTH_CONNECT);
            }
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED) {
                permissions.add(Manifest.permission.BLUETOOTH_SCAN);
            }
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED) {
                permissions.add(Manifest.permission.POST_NOTIFICATIONS);
            }
        }
        if (!permissions.isEmpty()) {
            ActivityCompat.requestPermissions(this, permissions.toArray(new String[0]), REQUEST_PERMISSIONS);
        } else {
            loadPairedDevices();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_PERMISSIONS) {
            boolean allGranted = true;
            for (int result : grantResults) {
                if (result != PackageManager.PERMISSION_GRANTED) {
                    allGranted = false;
                    break;
                }
            }
            if (allGranted) {
                loadPairedDevices();
            } else {
                Toast.makeText(this, "Se requieren permisos de Bluetooth", Toast.LENGTH_LONG).show();
            }
        }
    }

    private void loadPairedDevices() {
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        if (adapter == null) {
            Toast.makeText(this, "Bluetooth no disponible", Toast.LENGTH_LONG).show();
            return;
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                return;
            }
        }

        Set<BluetoothDevice> bonded = adapter.getBondedDevices();
        pairedDevices.clear();
        List<String> names = new ArrayList<>();

        for (BluetoothDevice device : bonded) {
            pairedDevices.add(device);
            names.add(device.getName() + "\n" + device.getAddress());
        }

        if (names.isEmpty()) {
            names.add("No hay dispositivos emparejados");
        }

        ArrayAdapter<String> adapterSpinner = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, names);
        adapterSpinner.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinnerDevices.setAdapter(adapterSpinner);

        if (serviceBound) {
            autoConnect();
        }
    }

    private void toggleConnection() {
        if (isConnecting) return;
        if (isConnected) {
            disconnect();
        } else {
            connectToSelectedDevice();
        }
    }

    private void connectToSelectedDevice() {
        int position = spinnerDevices.getSelectedItemPosition();
        if (position < 0 || position >= pairedDevices.size()) {
            Toast.makeText(this, "Seleccione un dispositivo", Toast.LENGTH_SHORT).show();
            return;
        }

        BluetoothDevice device = pairedDevices.get(position);
        String address = device.getAddress();

        Intent intent = new Intent(this, BluetoothService.class);
        intent.putExtra("device_address", address);
        startService(intent);

        if (serviceBound && bluetoothService != null) {
            bluetoothService.connectToDevice(address);
        }

        isConnecting = true;
        tvStatus.setText("Conectando...");
        tvStatus.setTextColor(getColor(android.R.color.holo_orange_dark));
        btnToggleConnection.setText("Desconectar");
    }

    private void disconnect() {
        if (serviceBound && bluetoothService != null) {
            bluetoothService.disconnect();
        }
        Intent intent = new Intent(this, BluetoothService.class);
        stopService(intent);
        isConnected = false;
        isConnecting = false;
        tvStatus.setText("Desconectado");
        tvStatus.setTextColor(getColor(android.R.color.holo_red_dark));
        btnToggleConnection.setText("Conectar");
    }

    private void bindService() {
        Intent intent = new Intent(this, BluetoothService.class);
        bindService(intent, serviceConnection, Context.BIND_AUTO_CREATE);
    }

    private void promptEnableBluetooth() {
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        if (adapter != null && !adapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_ENABLE_BT) {
            if (resultCode == RESULT_OK) {
                loadPairedDevices();
            } else {
                Toast.makeText(this, "Bluetooth debe estar encendido", Toast.LENGTH_LONG).show();
            }
        }
    }

    private void autoConnect() {
        if (!shouldAutoConnect || pairedDevices.isEmpty()) return;

        int hc05Index = -1;
        for (int i = 0; i < pairedDevices.size(); i++) {
            String name = pairedDevices.get(i).getName();
            if (name != null && name.toUpperCase().contains("HC-05")) {
                hc05Index = i;
                break;
            }
        }

        if (hc05Index >= 0) {
            spinnerDevices.setSelection(hc05Index);
            connectToSelectedDevice();
            shouldAutoConnect = false;
        }
    }

    private void sendThreshold() {
        String text = etThreshold.getText().toString().trim();
        if (text.isEmpty()) {
            Toast.makeText(this, "Ingrese un valor", Toast.LENGTH_SHORT).show();
            return;
        }
        try {
            float value = Float.parseFloat(text);
            if (value <= 0 || value > 100) {
                Toast.makeText(this, "Valor entre 0 y 100", Toast.LENGTH_SHORT).show();
                return;
            }
            String json = "{\"threshold\":" + value + "}";
            if (serviceBound && bluetoothService != null) {
                bluetoothService.sendCommand(json);
                Toast.makeText(this, "Umbral enviado: " + value, Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this, "No conectado", Toast.LENGTH_SHORT).show();
            }
        } catch (NumberFormatException e) {
            Toast.makeText(this, "Valor invalido", Toast.LENGTH_SHORT).show();
        }
    }

    private void activatePump() {
        if (!serviceBound || bluetoothService == null) {
            Toast.makeText(this, "No conectado", Toast.LENGTH_SHORT).show();
            return;
        }

        btnActivatePump.setEnabled(false);
        bluetoothService.sendCommand("{\"pump\":true}");
        Toast.makeText(this, "Bomba activada", Toast.LENGTH_SHORT).show();

        turnOffPumpRunnable = () -> {
            bluetoothService.sendCommand("{\"pump\":false}");
            Toast.makeText(this, "Bomba apagada - esperando cooldown", Toast.LENGTH_SHORT).show();
            cooldownRunnable = () -> {
                btnActivatePump.setEnabled(true);
            };
            pumpHandler.postDelayed(cooldownRunnable, PUMP_COOLDOWN_MS);
        };

        pumpHandler.postDelayed(turnOffPumpRunnable, PUMP_ON_DURATION_MS);
    }
}
