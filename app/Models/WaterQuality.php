<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\Factories\HasFactory;

class WaterQuality extends Model
{
    use HasFactory;

    protected $table = 'water_quality';

    protected $fillable = [
        'device_id',
        'turbidity',
        'tds',
        'ph',
        'status',
        'alert_level',
        'location',
        'raw_turbidity_voltage',
        'raw_tds_voltage',
        'raw_ph_voltage'
    ];

    protected $casts = [
        'turbidity' => 'float',
        'tds' => 'float',
        'ph' => 'float',
        'raw_turbidity_voltage' => 'float',
        'raw_tds_voltage' => 'float',
        'raw_ph_voltage' => 'float',
        'created_at' => 'datetime',
        'updated_at' => 'datetime',
    ];

    /**
     * Scope to filter by device
     */
    public function scopeByDevice($query, $deviceId)
    {
        return $query->where('device_id', $deviceId);
    }

    /**
     * Scope to get recent readings
     */
    public function scopeRecent($query, $hours = 24)
    {
        return $query->where('created_at', '>=', now()->subHours($hours));
    }

    /**
     * Scope to get readings with alerts
     */
    public function scopeWithAlerts($query)
    {
        return $query->whereIn('alert_level', ['warning', 'critical']);
    }

    /**
     * Determine if the reading is within safe parameters
     */
    public function isSafe()
    {
        return $this->alert_level === 'normal';
    }

    /**
     * Get alert level based on sensor values
     */
    public static function determineAlertLevel($turbidity, $tds, $ph)
    {
        // Define thresholds (matching Arduino code)
        $TDS_THRESHOLD = 500.0;     // ppm
        $TURB_THRESHOLD = 2.32;     // V (below = turbid)
        $PH_LOW = 6.5;              // WHO safe range
        $PH_HIGH = 8.5;

        $alerts = [];

        // Check TDS
        if ($tds > $TDS_THRESHOLD) {
            $alerts[] = 'high_tds';
        }

        // Check turbidity (voltage below threshold means turbid water)
        if ($turbidity < $TURB_THRESHOLD) {
            $alerts[] = 'turbid_water';
        }

        // Check pH
        if ($ph < $PH_LOW || $ph > $PH_HIGH) {
            $alerts[] = 'ph_out_of_range';
        }

        // Determine overall alert level
        if (empty($alerts)) {
            return 'normal';
        } elseif (count($alerts) >= 2) {
            return 'critical';
        } else {
            return 'warning';
        }
    }

    /**
     * Get human-readable status description
     */
    public function getStatusDescriptionAttribute()
    {
        $descriptions = [];

        if ($this->tds > 500) {
            $descriptions[] = "High TDS ({$this->tds} ppm)";
        }

        if ($this->turbidity < 2.32) {
            $descriptions[] = "Turbid water ({$this->turbidity}V)";
        }

        if ($this->ph < 6.5 || $this->ph > 8.5) {
            $descriptions[] = "pH out of range ({$this->ph})";
        }

        return empty($descriptions) ? 'All parameters normal' : implode(', ', $descriptions);
    }
}