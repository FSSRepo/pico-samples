package com.steward.riego;

import android.graphics.Color;
import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;

import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.github.mikephil.charting.formatter.ValueFormatter;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.List;
import java.util.Locale;

public class HistoryActivity extends AppCompatActivity {

    private LineChart lineChart;
    private DatabaseHelper db;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_history);

        lineChart = findViewById(R.id.lineChart);
        db = new DatabaseHelper(this);

        setupChart();
        loadChartData();
    }

    private void setupChart() {
        lineChart.getDescription().setEnabled(false);
        lineChart.setTouchEnabled(true);
        lineChart.setDragEnabled(true);
        lineChart.setScaleEnabled(true);
        lineChart.setPinchZoom(true);
        lineChart.getLegend().setEnabled(true);
        lineChart.setExtraOffsets(8f, 8f, 16f, 8f);

        XAxis xAxis = lineChart.getXAxis();
        xAxis.setPosition(XAxis.XAxisPosition.BOTTOM);
        xAxis.setValueFormatter(new ValueFormatter() {
            private final SimpleDateFormat sdf = new SimpleDateFormat("HH:mm", Locale.getDefault());
            @Override
            public String getFormattedValue(float value) {
                return sdf.format(new Date((long) value));
            }
        });
        xAxis.setLabelRotationAngle(45f);
        xAxis.setGranularity(1f);
        xAxis.setAvoidFirstLastClipping(true);

        YAxis leftAxis = lineChart.getAxisLeft();
        leftAxis.setAxisMinimum(0f);
        leftAxis.setAxisMaximum(60f);
        leftAxis.setLabelCount(6, true);

        YAxis rightAxis = lineChart.getAxisRight();
        rightAxis.setEnabled(true);
        rightAxis.setAxisMinimum(0f);
        rightAxis.setAxisMaximum(100f);
        rightAxis.setLabelCount(6, true);
    }

    private void loadChartData() {
        List<SensorReading> readings = db.getReadings(100);
        Collections.sort(readings, new Comparator<SensorReading>() {
            @Override
            public int compare(SensorReading a, SensorReading b) {
                return Long.compare(a.getTimestamp(), b.getTimestamp());
            }
        });

        if (readings.isEmpty()) {
            lineChart.clear();
            lineChart.invalidate();
            return;
        }

        ArrayList<Entry> tempEntries = new ArrayList<>();
        ArrayList<Entry> humEntries = new ArrayList<>();

        for (SensorReading r : readings) {
            tempEntries.add(new Entry(r.getTimestamp(), r.getTemperature()));
            humEntries.add(new Entry(r.getTimestamp(), r.getHumidity()));
        }

        LineDataSet tempDataSet = new LineDataSet(tempEntries, "Temperatura (°C)");
        tempDataSet.setColor(Color.parseColor("#2E7D32"));
        tempDataSet.setCircleColor(Color.parseColor("#2E7D32"));
        tempDataSet.setLineWidth(2f);
        tempDataSet.setCircleRadius(3f);
        tempDataSet.setDrawValues(false);
        tempDataSet.setAxisDependency(YAxis.AxisDependency.LEFT);

        LineDataSet humDataSet = new LineDataSet(humEntries, "Humedad (%)");
        humDataSet.setColor(Color.parseColor("#1565C0"));
        humDataSet.setCircleColor(Color.parseColor("#1565C0"));
        humDataSet.setLineWidth(2f);
        humDataSet.setCircleRadius(3f);
        humDataSet.setDrawValues(false);
        humDataSet.setAxisDependency(YAxis.AxisDependency.RIGHT);

        LineData lineData = new LineData(tempDataSet, humDataSet);
        lineChart.setData(lineData);

        lineChart.getAxisLeft().setAxisMaximum(Math.max(60f, getMaxValue(tempEntries) * 1.1f));
        lineChart.getAxisRight().setAxisMaximum(Math.max(100f, getMaxValue(humEntries) * 1.1f));

        lineChart.invalidate();
    }

    private float getMaxValue(ArrayList<Entry> entries) {
        float max = 0f;
        for (Entry e : entries) {
            if (e.getY() > max) max = e.getY();
        }
        return max;
    }
}
