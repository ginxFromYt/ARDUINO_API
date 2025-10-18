<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use App\Models\WaterQuality;
use Illuminate\Support\Facades\Auth;

class WaterQualityController extends Controller
{
    /**
     * Display the water quality monitoring dashboard
     */
    public function dashboard()
    {
        // Get latest readings from all devices
        $latestReadings = WaterQuality::select('device_id')
            ->selectRaw('MAX(created_at) as latest_time')
            ->groupBy('device_id')
            ->get()
            ->map(function ($group) {
                return WaterQuality::where('device_id', $group->device_id)
                    ->where('created_at', $group->latest_time)
                    ->first();
            })->filter();

        // Get recent alert data (last 24 hours)
        $recentAlerts = WaterQuality::whereIn('alert_level', ['warning', 'critical'])
            ->where('created_at', '>=', now()->subHours(24))
            ->orderBy('created_at', 'desc')
            ->limit(10)
            ->get();

        // Get statistics
        $stats = [
            'total_devices' => WaterQuality::distinct('device_id')->count(),
            'total_readings' => WaterQuality::count(),
            'alerts_today' => WaterQuality::whereIn('alert_level', ['warning', 'critical'])
                ->whereDate('created_at', today())
                ->count(),
            'online_devices' => WaterQuality::where('created_at', '>=', now()->subMinutes(10))
                ->distinct('device_id')
                ->count(),
        ];

        return view('water-quality.dashboard', compact('latestReadings', 'recentAlerts', 'stats'));
    }

    /**
     * Display device details and historical data
     */
    public function device(Request $request, $deviceId)
    {
        $device = WaterQuality::where('device_id', $deviceId)->first();
        
        if (!$device) {
            abort(404, 'Device not found');
        }

        // Get recent readings (last 24 hours by default)
        $hours = $request->get('hours', 24);
        $readings = WaterQuality::where('device_id', $deviceId)
            ->where('created_at', '>=', now()->subHours($hours))
            ->orderBy('created_at', 'desc')
            ->paginate(50);

        // Get device statistics
        $deviceStats = [
            'latest_reading' => WaterQuality::where('device_id', $deviceId)
                ->latest()
                ->first(),
            'total_readings' => WaterQuality::where('device_id', $deviceId)->count(),
            'alerts_count' => WaterQuality::where('device_id', $deviceId)
                ->whereIn('alert_level', ['warning', 'critical'])
                ->count(),
            'last_online' => WaterQuality::where('device_id', $deviceId)
                ->latest()
                ->value('created_at'),
        ];

        return view('water-quality.device', compact('deviceId', 'readings', 'deviceStats', 'hours'));
    }

    /**
     * Display all devices list
     */
    public function devices()
    {
        $devices = WaterQuality::select('device_id')
            ->selectRaw('
                MAX(created_at) as last_seen,
                COUNT(*) as total_readings,
                SUM(CASE WHEN alert_level IN ("warning", "critical") THEN 1 ELSE 0 END) as alert_count,
                MAX(location) as location
            ')
            ->groupBy('device_id')
            ->orderBy('last_seen', 'desc')
            ->get()
            ->map(function ($device) {
                $latest = WaterQuality::where('device_id', $device->device_id)
                    ->latest()
                    ->first();
                
                $device->latest_reading = $latest;
                $device->is_online = $latest && $latest->created_at->isAfter(now()->subMinutes(10));
                
                return $device;
            });

        return view('water-quality.devices', compact('devices'));
    }

    /**
     * Get chart data for device (AJAX endpoint)
     */
    public function chartData(Request $request, $deviceId)
    {
        $hours = $request->get('hours', 24);
        
        $readings = WaterQuality::where('device_id', $deviceId)
            ->where('created_at', '>=', now()->subHours($hours))
            ->orderBy('created_at', 'asc')
            ->get(['created_at', 'turbidity', 'tds', 'ph', 'alert_level']);

        $chartData = [
            'labels' => $readings->pluck('created_at')->map(function ($date) {
                return $date->format('M j, H:i');
            }),
            'datasets' => [
                [
                    'label' => 'Turbidity (V)',
                    'data' => $readings->pluck('turbidity'),
                    'borderColor' => 'rgb(59, 130, 246)',
                    'backgroundColor' => 'rgba(59, 130, 246, 0.1)',
                    'yAxisID' => 'y'
                ],
                [
                    'label' => 'TDS (ppm)',
                    'data' => $readings->pluck('tds'),
                    'borderColor' => 'rgb(34, 197, 94)',
                    'backgroundColor' => 'rgba(34, 197, 94, 0.1)',
                    'yAxisID' => 'y1'
                ],
                [
                    'label' => 'pH',
                    'data' => $readings->pluck('ph'),
                    'borderColor' => 'rgb(239, 68, 68)',
                    'backgroundColor' => 'rgba(239, 68, 68, 0.1)',
                    'yAxisID' => 'y2'
                ]
            ]
        ];

        return response()->json($chartData);
    }

    /**
     * Export data as CSV
     */
    public function export(Request $request, $deviceId)
    {
        $hours = $request->get('hours', 24);
        
        $readings = WaterQuality::where('device_id', $deviceId)
            ->where('created_at', '>=', now()->subHours($hours))
            ->orderBy('created_at', 'desc')
            ->get();

        $filename = "water_quality_{$deviceId}_" . now()->format('Y-m-d_H-i-s') . '.csv';

        $headers = [
            'Content-Type' => 'text/csv',
            'Content-Disposition' => "attachment; filename=\"$filename\"",
        ];

        $callback = function () use ($readings) {
            $file = fopen('php://output', 'w');
            
            // CSV headers
            fputcsv($file, [
                'Timestamp',
                'Device ID',
                'Turbidity (V)',
                'TDS (ppm)',
                'pH',
                'Alert Level',
                'Status Description',
                'Location'
            ]);

            // CSV data
            foreach ($readings as $reading) {
                fputcsv($file, [
                    $reading->created_at->format('Y-m-d H:i:s'),
                    $reading->device_id,
                    $reading->turbidity,
                    $reading->tds,
                    $reading->ph,
                    $reading->alert_level,
                    $reading->status_description,
                    $reading->location ?? 'N/A'
                ]);
            }

            fclose($file);
        };

        return response()->stream($callback, 200, $headers);
    }
}
