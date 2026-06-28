package com.steward.riego;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.PowerManager;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.core.app.NotificationCompat;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.io.InputStream;
import java.util.UUID;

public class BluetoothService extends Service {
    private static final String TAG = "BluetoothService";
    private static final String SERVICE_CHANNEL_ID = "pump_service_channel";
    private static final String ALERT_CHANNEL_ID = "pump_alert_channel";
    private static final int SERVICE_NOTIFICATION_ID = 1;
    private static final int ALERT_NOTIFICATION_ID = 2;
    private static final UUID SPP_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    private final IBinder binder = new LocalBinder();
    private BluetoothAdapter bluetoothAdapter;
    private BluetoothSocket socket;
    private Thread connectThread;
    private volatile boolean running = false;
    private PowerManager.WakeLock wakeLock;

    private String deviceAddress = null;
    private BluetoothListener listener;
    private boolean isForeground = false;
    private boolean lastPumpOn = false;

    public interface BluetoothListener {
        void onDataReceived(float temp, float hum, boolean pumpOn);
        void onConnectionStateChanged(boolean connected);
        void onError(String error);
    }

    public class LocalBinder extends Binder {
        BluetoothService getService() {
            return BluetoothService.this;
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        createNotificationChannels();
        acquireWakeLock();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (!isForeground) {
            startForeground(SERVICE_NOTIFICATION_ID, buildServiceNotification());
            isForeground = true;
        }
        if (intent != null && intent.hasExtra("device_address")) {
            deviceAddress = intent.getStringExtra("device_address");
            connect();
        }
        return START_STICKY;
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return binder;
    }

    @Override
    public void onDestroy() {
        running = false;
        closeSocket();
        stopForegroundIfNeeded();
        cancelAlertNotification();
        if (wakeLock != null && wakeLock.isHeld()) {
            wakeLock.release();
        }
        super.onDestroy();
    }

    public void setListener(BluetoothListener listener) {
        this.listener = listener;
    }

    public void connectToDevice(String address) {
        this.deviceAddress = address;
        connect();
    }

    public void disconnect() {
        running = false;
        closeSocket();
    }

    public void sendCommand(String command) {
        if (socket != null && socket.isConnected()) {
            try {
                socket.getOutputStream().write((command + "\n").getBytes());
            } catch (IOException e) {
                Log.e(TAG, "Send error", e);
                notifyError("Error enviando comando");
            }
        } else {
            notifyError("No conectado");
        }
    }

    private void connect() {
        if (connectThread != null && connectThread.isAlive()) {
            return;
        }
        running = true;
        connectThread = new Thread(this::connectLoop);
        connectThread.start();
    }

    private void connectLoop() {
        while (running) {
            if (deviceAddress == null || bluetoothAdapter == null) {
                notifyError("Bluetooth no disponible");
                break;
            }

            BluetoothDevice device = bluetoothAdapter.getRemoteDevice(deviceAddress);
            closeSocket();

            try {
                socket = device.createRfcommSocketToServiceRecord(SPP_UUID);
                socket.connect();
                notifyConnectionState(true);
                lastPumpOn = false;
                readLoop();
            } catch (IOException | SecurityException e) {
                Log.e(TAG, "Connection failed", e);
                notifyConnectionState(false);
                stopForegroundIfNeeded();
                lastPumpOn = false;
                try {
                    Thread.sleep(3000);
                } catch (InterruptedException ignored) {
                }
            }
        }
    }

    private void readLoop() {
        try {
            InputStream inputStream = socket.getInputStream();
            byte[] buffer = new byte[256];
            StringBuilder lineBuffer = new StringBuilder();

            while (running && socket != null && socket.isConnected()) {
                int bytesRead = inputStream.read(buffer);
                if (bytesRead > 0) {
                    String chunk = new String(buffer, 0, bytesRead);
                    lineBuffer.append(chunk);

                    int newlineIndex;
                    while ((newlineIndex = lineBuffer.indexOf("\n")) != -1) {
                        String line = lineBuffer.substring(0, newlineIndex).trim();
                        lineBuffer.delete(0, newlineIndex + 1);
                        parseAndNotify(line);
                    }
                }
            }
        } catch (IOException e) {
            Log.e(TAG, "Read error", e);
            notifyConnectionState(false);
            stopForegroundIfNeeded();
            lastPumpOn = false;
        }
    }

    private void parseAndNotify(String line) {
        try {
            JSONObject json = new JSONObject(line);
            float temp = json.has("temp") ? (float) json.getDouble("temp") : 0.0f;
            float hum = json.has("hum") ? (float) json.getDouble("hum") : 0.0f;
            boolean pumpOn = json.has("pump") ? json.getBoolean("pump") : false;
            notifyData(temp, hum, pumpOn);
        } catch (JSONException e) {
            Log.w(TAG, "Invalid JSON: " + line);
        }
    }

    private void notifyData(float temp, float hum, boolean pumpOn) {
        if (listener != null) {
            new Handler(Looper.getMainLooper()).post(() -> listener.onDataReceived(temp, hum, pumpOn));
        }
        if (pumpOn != lastPumpOn) {
            if (pumpOn) {
                sendAlertNotification("Bomba de agua ENCENDIDA",
                        String.format("Temp: %.1f°C  Hum: %.1f%%", temp, hum));
            } else {
                cancelAlertNotification();
            }
            lastPumpOn = pumpOn;
        }
    }

    private void notifyConnectionState(boolean connected) {
        if (listener != null) {
            new Handler(Looper.getMainLooper()).post(() -> listener.onConnectionStateChanged(connected));
        }
    }

    private void notifyError(String error) {
        if (listener != null) {
            new Handler(Looper.getMainLooper()).post(() -> listener.onError(error));
        }
    }

    private void closeSocket() {
        if (socket != null) {
            try {
                socket.close();
            } catch (IOException ignored) {
            }
            socket = null;
        }
    }

    private void createNotificationChannels() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationManager manager = getSystemService(NotificationManager.class);
            if (manager == null) return;

            NotificationChannel serviceChannel = new NotificationChannel(
                    SERVICE_CHANNEL_ID, "Servicio de monitoreo", NotificationManager.IMPORTANCE_LOW);
            serviceChannel.setDescription("Mantiene la conexión Bluetooth activa");
            serviceChannel.setSound(null, null);
            serviceChannel.enableVibration(false);

            NotificationChannel alertChannel = new NotificationChannel(
                    ALERT_CHANNEL_ID, "Alertas de bomba", NotificationManager.IMPORTANCE_HIGH);
            alertChannel.setDescription("Notificaciones cuando la bomba se enciende");
            alertChannel.enableVibration(true);

            manager.createNotificationChannel(serviceChannel);
            manager.createNotificationChannel(alertChannel);
        }
    }

    private Notification buildServiceNotification() {
        Intent intent = new Intent(this, MainActivity.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(
                this, 0, intent, PendingIntent.FLAG_IMMUTABLE);

        return new NotificationCompat.Builder(this, SERVICE_CHANNEL_ID)
                .setContentTitle("Monitor de Bomba")
                .setContentText("Conectado - Esperando datos...")
                .setSmallIcon(android.R.drawable.ic_dialog_info)
                .setContentIntent(pendingIntent)
                .setOngoing(true)
                .setSilent(true)
                .build();
    }

    private void sendAlertNotification(String title, String content) {
        Intent intent = new Intent(this, MainActivity.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(
                this, 0, intent, PendingIntent.FLAG_IMMUTABLE);

        Notification notification = new NotificationCompat.Builder(this, ALERT_CHANNEL_ID)
                .setContentTitle(title)
                .setContentText(content)
                .setSmallIcon(android.R.drawable.ic_dialog_alert)
                .setContentIntent(pendingIntent)
                .setAutoCancel(true)
                .setPriority(NotificationCompat.PRIORITY_HIGH)
                .setCategory(NotificationCompat.CATEGORY_ALARM)
                .build();

        NotificationManager manager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        if (manager != null) {
            manager.notify(ALERT_NOTIFICATION_ID, notification);
        }
    }

    private void cancelAlertNotification() {
        NotificationManager manager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        if (manager != null) {
            manager.cancel(ALERT_NOTIFICATION_ID);
        }
    }

    private void stopForegroundIfNeeded() {
        if (isForeground) {
            stopForeground(true);
            isForeground = false;
        }
    }

    private void acquireWakeLock() {
        PowerManager powerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
        if (powerManager != null) {
            wakeLock = powerManager.newWakeLock(
                    PowerManager.PARTIAL_WAKE_LOCK, "BluetoothService::WakeLock");
            wakeLock.acquire();
        }
    }
}
