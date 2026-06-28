package com.steward.riego;

public class SensorReading {
    private long id;
    private float temperature;
    private float humidity;
    private boolean pumpOn;
    private long timestamp;

    public SensorReading() {}

    public SensorReading(float temperature, float humidity, boolean pumpOn, long timestamp) {
        this.temperature = temperature;
        this.humidity = humidity;
        this.pumpOn = pumpOn;
        this.timestamp = timestamp;
    }

    public long getId() { return id; }
    public void setId(long id) { this.id = id; }

    public float getTemperature() { return temperature; }
    public void setTemperature(float temperature) { this.temperature = temperature; }

    public float getHumidity() { return humidity; }
    public void setHumidity(float humidity) { this.humidity = humidity; }

    public boolean isPumpOn() { return pumpOn; }
    public void setPumpOn(boolean pumpOn) { this.pumpOn = pumpOn; }

    public long getTimestamp() { return timestamp; }
    public void setTimestamp(long timestamp) { this.timestamp = timestamp; }
}
