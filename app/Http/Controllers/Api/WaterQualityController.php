<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use Illuminate\Http\Request;
use App\Models\WaterQuality;
use Illuminate\Support\Facades\Validator;
use Illuminate\Support\Facades\Log;

class WaterQualityController extends Controller
{
    /**
     * Store water quality data from Arduino device
     */
    public function store(Request $request)
    {
        try {
            // Validate incoming data
            $validator = Validator::make($request->all(), [
                'device_id' => 'required|string|max:50',
                'turbidity' => 'required|numeric|min:0|max:5', // Voltage range 0-5V
                'tds' => 'required|numeric|min:0|max:9999', // TDS in ppm
                'ph' => 'required|numeric|min:0|max:14', // pH range 0-14
                'location' => 'nullable|string|max:255',
                'raw_turbidity_voltage' => 'nullable|numeric|min:0|max:5',
                'raw_tds_voltage' => 'nullable|numeric|min:0|max:5',
                'raw_ph_voltage' => 'nullable|numeric|min:0|max:5',
                'sensor_states' => 'nullable|array',
                'alert_message' => 'nullable|string|max:1000'
            ]);

            if ($validator->fails()) {
                return response()->json([
                    'success' => false,
                    'message' => 'Validation failed',
                    'errors' => $validator->errors()
                ], 422);
            }

            $data = $validator->validated();

            // Determine alert level based on sensor values
            $alertLevel = WaterQuality::determineAlertLevel(
                $data['turbidity'],
                $data['tds'],
                $data['ph']
            );

            // Create water quality record
            $waterQuality = WaterQuality::create([
                'device_id' => $data['device_id'],
                'turbidity' => $data['turbidity'],
                'tds' => $data['tds'],
                'ph' => $data['ph'],
                'status' => $alertLevel,
                'alert_level' => $alertLevel,
                'location' => $data['location'] ?? null,
                'raw_turbidity_voltage' => $data['raw_turbidity_voltage'] ?? $data['turbidity'],
                'raw_tds_voltage' => $data['raw_tds_voltage'] ?? null,
                'raw_ph_voltage' => $data['raw_ph_voltage'] ?? null,
                'sensor_states' => $data['sensor_states'] ?? null,
                'alert_message' => $data['alert_message'] ?? null
            ]);

            // Log for monitoring
            Log::info('Water quality data received', [
                'device_id' => $data['device_id'],
                'alert_level' => $alertLevel,
                'turbidity' => $data['turbidity'],
                'tds' => $data['tds'],
                'ph' => $data['ph']
            ]);

            return response()->json([
                'success' => true,
                'message' => 'Water quality data stored successfully',
                'data' => [
                    'id' => $waterQuality->id,
                    'device_id' => $waterQuality->device_id,
                    'alert_level' => $waterQuality->alert_level,
                    'status_description' => $waterQuality->status_description,
                    'timestamp' => $waterQuality->created_at->toISOString()
                ]
            ], 201);

        } catch (\Exception $e) {
            Log::error('Error storing water quality data', [
                'message' => $e->getMessage(),
                'request_data' => $request->all()
            ]);

            return response()->json([
                'success' => false,
                'message' => 'Internal server error',
                'error' => config('app.debug') ? $e->getMessage() : 'Something went wrong'
            ], 500);
        }
    }

    /**
     * Get latest water quality data for a device
     */
    public function getLatest(Request $request)
    {
        $validator = Validator::make($request->all(), [
            'device_id' => 'required|string|max:50'
        ]);

        if ($validator->fails()) {
            return response()->json([
                'success' => false,
                'message' => 'Validation failed',
                'errors' => $validator->errors()
            ], 422);
        }

        $latest = WaterQuality::byDevice($request->device_id)
            ->latest()
            ->first();

        if (!$latest) {
            return response()->json([
                'success' => false,
                'message' => 'No data found for this device'
            ], 404);
        }

        return response()->json([
            'success' => true,
            'data' => [
                'id' => $latest->id,
                'device_id' => $latest->device_id,
                'turbidity' => $latest->turbidity,
                'tds' => $latest->tds,
                'ph' => $latest->ph,
                'alert_level' => $latest->alert_level,
                'status_description' => $latest->status_description,
                'location' => $latest->location,
                'timestamp' => $latest->created_at->toISOString()
            ]
        ]);
    }

    /**
     * Get recent water quality data for a device
     */
    public function getRecent(Request $request)
    {
        $validator = Validator::make($request->all(), [
            'device_id' => 'required|string|max:50',
            'hours' => 'nullable|integer|min:1|max:168' // Max 1 week
        ]);

        if ($validator->fails()) {
            return response()->json([
                'success' => false,
                'message' => 'Validation failed',
                'errors' => $validator->errors()
            ], 422);
        }

        $hours = $request->get('hours', 24);

        $readings = WaterQuality::byDevice($request->device_id)
            ->recent($hours)
            ->orderBy('created_at', 'desc')
            ->get()
            ->map(function ($reading) {
                return [
                    'id' => $reading->id,
                    'turbidity' => $reading->turbidity,
                    'tds' => $reading->tds,
                    'ph' => $reading->ph,
                    'alert_level' => $reading->alert_level,
                    'status_description' => $reading->status_description,
                    'timestamp' => $reading->created_at->toISOString()
                ];
            });

        return response()->json([
            'success' => true,
            'data' => $readings,
            'total' => $readings->count(),
            'period_hours' => $hours
        ]);
    }

    /**
     * Get device status/health check
     */
    public function deviceStatus(Request $request)
    {
        $validator = Validator::make($request->all(), [
            'device_id' => 'required|string|max:50'
        ]);

        if ($validator->fails()) {
            return response()->json([
                'success' => false,
                'message' => 'Validation failed',
                'errors' => $validator->errors()
            ], 422);
        }

        $deviceId = $request->device_id;
        
        // Get latest reading
        $latest = WaterQuality::byDevice($deviceId)->latest()->first();
        
        if (!$latest) {
            return response()->json([
                'success' => true,
                'device_status' => 'no_data',
                'message' => 'No data received from device yet'
            ]);
        }

        // Check if device is online (received data in last 10 minutes)
        $isOnline = $latest->created_at->isAfter(now()->subMinutes(10));
        
        // Get alert summary for last 24 hours
        $alertSummary = WaterQuality::byDevice($deviceId)
            ->recent(24)
            ->selectRaw('alert_level, COUNT(*) as count')
            ->groupBy('alert_level')
            ->pluck('count', 'alert_level')
            ->toArray();

        return response()->json([
            'success' => true,
            'device_status' => $isOnline ? 'online' : 'offline',
            'last_seen' => $latest->created_at->toISOString(),
            'current_alert_level' => $latest->alert_level,
            'alert_summary_24h' => $alertSummary,
            'total_readings' => WaterQuality::byDevice($deviceId)->count()
        ]);
    }
}