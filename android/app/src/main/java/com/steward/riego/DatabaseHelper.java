package com.steward.riego;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import java.util.ArrayList;
import java.util.List;

public class DatabaseHelper extends SQLiteOpenHelper {
    private static final String DATABASE_NAME = "riego.db";
    private static final int DATABASE_VERSION = 1;

    private static final String TABLE_READINGS = "readings";
    private static final String COLUMN_ID = "id";
    private static final String COLUMN_TEMP = "temperature";
    private static final String COLUMN_HUM = "humidity";
    private static final String COLUMN_PUMP = "pump_on";
    private static final String COLUMN_TIME = "timestamp";

    private static final String CREATE_TABLE =
            "CREATE TABLE " + TABLE_READINGS + " (" +
            COLUMN_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
            COLUMN_TEMP + " REAL, " +
            COLUMN_HUM + " REAL, " +
            COLUMN_PUMP + " INTEGER, " +
            COLUMN_TIME + " INTEGER)";

    public DatabaseHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        db.execSQL(CREATE_TABLE);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        db.execSQL("DROP TABLE IF EXISTS " + TABLE_READINGS);
        onCreate(db);
    }

    public long insertReading(SensorReading reading) {
        SQLiteDatabase db = this.getWritableDatabase();
        ContentValues values = new ContentValues();
        values.put(COLUMN_TEMP, reading.getTemperature());
        values.put(COLUMN_HUM, reading.getHumidity());
        values.put(COLUMN_PUMP, reading.isPumpOn() ? 1 : 0);
        values.put(COLUMN_TIME, reading.getTimestamp());
        return db.insert(TABLE_READINGS, null, values);
    }

    public List<SensorReading> getReadings(int limit) {
        List<SensorReading> readings = new ArrayList<>();
        String query = "SELECT * FROM " + TABLE_READINGS + " ORDER BY " + COLUMN_TIME + " DESC LIMIT ?";
        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery(query, new String[]{String.valueOf(limit)});

        if (cursor.moveToFirst()) {
            do {
                SensorReading r = new SensorReading();
                r.setId(cursor.getLong(0));
                r.setTemperature(cursor.getFloat(1));
                r.setHumidity(cursor.getFloat(2));
                r.setPumpOn(cursor.getInt(3) == 1);
                r.setTimestamp(cursor.getLong(4));
                readings.add(r);
            } while (cursor.moveToNext());
        }
        cursor.close();
        return readings;
    }

    public List<SensorReading> getReadingsInRange(long startTime, long endTime) {
        List<SensorReading> readings = new ArrayList<>();
        String query = "SELECT * FROM " + TABLE_READINGS +
                " WHERE " + COLUMN_TIME + " >= ? AND " + COLUMN_TIME + " <= ?" +
                " ORDER BY " + COLUMN_TIME + " ASC";
        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery(query, new String[]{String.valueOf(startTime), String.valueOf(endTime)});

        if (cursor.moveToFirst()) {
            do {
                SensorReading r = new SensorReading();
                r.setId(cursor.getLong(0));
                r.setTemperature(cursor.getFloat(1));
                r.setHumidity(cursor.getFloat(2));
                r.setPumpOn(cursor.getInt(3) == 1);
                r.setTimestamp(cursor.getLong(4));
                readings.add(r);
            } while (cursor.moveToNext());
        }
        cursor.close();
        return readings;
    }

    public void deleteOldReadings(long olderThanTimestamp) {
        SQLiteDatabase db = this.getWritableDatabase();
        db.delete(TABLE_READINGS, COLUMN_TIME + " < ?", new String[]{String.valueOf(olderThanTimestamp)});
    }
}
