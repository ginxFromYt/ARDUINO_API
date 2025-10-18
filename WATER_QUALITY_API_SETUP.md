# Water Quality Monitoring API Setup Guide

## Overview
This system allows your Arduino water quality monitoring device (with TDS, Turbidity, and pH sensors) to send data to your Laravel API via GPRS/2G connection using SIM800A module.

## What's Been Created

### 1. Database Structure
- **Table**: `water_quality`
- **Migration**: `2025_10_18_042035_create_water_quality_table.php`
- **Fields**:
  - `device_id`: Unique identifier for your Arduino device
  - `turbidity`: Turbidity voltage reading
  - `tds`: TDS value in ppm
  - `ph`: pH value
  - `status` & `alert_level`: Calculated based on sensor thresholds
  - `location`: Optional device location
  - Raw voltage readings for debugging
  - `sensor_states`: JSON field storing alert states
  - `alert_message`: Human-readable alert description

### 2. Laravel API Components
- **Model**: `app/Models/WaterQuality.php`
- **Controller**: `app/Http/Controllers/Api/WaterQualityController.php`
- **Routes**: Added to `routes/api.php`

### 3. API Endpoints

#### For Arduino (requires API key):
- `POST /api/water-quality/data` - Store sensor data
- `GET /api/water-quality/status?device_id=WQM001` - Device health check

#### Public endpoints (for monitoring dashboard):
- `GET /api/water-quality/latest?device_id=WQM001` - Get latest reading
- `GET /api/water-quality/recent?device_id=WQM001&hours=24` - Get recent readings

### 4. Arduino Code
- **File**: `arduino_water_quality_with_api.ino`
- Enhanced version of your original code with GPRS HTTP POST functionality

## Setup Instructions

### 1. Laravel Configuration

#### Middleware Configuration
✅ **Already configured!** The `api.key` middleware is properly set up:

- **Middleware class**: `app/Http/Middleware/CheckApiKey.php`
- **Registration**: `bootstrap/app.php` 
- **Features**:
  - Supports multiple header formats: `X-API-KEY`, `API-KEY`, `Authorization`
  - Handles Bearer token format
  - Uses secure `hash_equals()` to prevent timing attacks
  - Returns structured JSON error responses

**Supported header formats:**
```bash
X-API-KEY: your-api-key
API-KEY: your-api-key  
Authorization: Bearer your-api-key
Authorization: your-api-key
```

#### Add API key to your environment
✅ **Configuration ready!** Add to your `.env` file:
```bash
API_KEY=your-secure-api-key-here
```

**Generate a secure API key:**
```bash
# Option 1: Using Laravel tinker
php artisan tinker
>>> Str::random(32)

# Option 2: Using openssl
openssl rand -base64 32
```

✅ **Config already added** to `config/app.php`:
```php
'api_key' => env('API_KEY'),
```

### 2. Arduino Configuration

Update the following variables in the Arduino code:

```cpp
// API Configuration
String apiHost = "yourdomain.com";        // Your domain (without http://)
String apiPath = "/api/water-quality/data";
String apiKey = "your-secure-api-key";    // Same as in .env
String deviceId = "WQM001";               // Unique device ID

// GPRS Configuration
String apnName = "internet";              // Your carrier's APN
String apnUser = "";                      // APN username (if required)
String apnPass = "";                      // APN password (if required)
```

### 3. Hardware Connections

#### SIM800A Module:
- VCC → 3.7-4.2V (external power supply recommended)
- GND → Arduino GND
- TXD → Arduino pin 7 (RX)
- RXD → Arduino pin 8 (TX)

#### Sensors (same as your original setup):
- Turbidity sensor → A0
- TDS sensor → A1
- pH sensor → A2
- LCD (I2C) → SDA/SCL pins

### 4. Testing the API

#### Test with curl:
```bash
curl -X POST "http://yourdomain.com/api/water-quality/data" \
  -H "Content-Type: application/json" \
  -H "X-API-KEY: your-api-key" \
  -d '{
    "device_id": "WQM001",
    "turbidity": 3.2,
    "tds": 250.5,
    "ph": 7.1,
    "location": "Test Location"
  }'
```

#### Check latest data:
```bash
curl "http://yourdomain.com/api/water-quality/latest?device_id=WQM001"
```

## Features

### Arduino Features:
- ✅ Original SMS alerts (maintained)
- ✅ LCD display with GPRS status
- ✅ Automatic GPRS connection and reconnection
- ✅ Regular data transmission (every 5 minutes)
- ✅ Immediate alert transmission (30 seconds after alert)
- ✅ Fallback to SMS-only mode if GPRS fails
- ✅ JSON payload with all sensor data and states

### API Features:
- ✅ Automatic alert level calculation
- ✅ Data validation and error handling
- ✅ Device status monitoring
- ✅ Historical data queries
- ✅ Comprehensive logging
- ✅ RESTful API design

### Database Features:
- ✅ Efficient indexing for fast queries
- ✅ Raw voltage storage for debugging
- ✅ JSON sensor states storage
- ✅ Alert message storage
- ✅ Timestamp tracking

## Data Transmission Schedule

1. **Regular data**: Every 5 minutes
2. **Alert data**: 30 seconds after alert condition is detected
3. **SMS alerts**: Immediate (as per original code)
4. **Recovery notifications**: Immediate (both SMS and API)

## Monitoring and Troubleshooting

### Check device status:
```bash
curl "http://yourdomain.com/api/water-quality/status?device_id=WQM001"
```

### View Laravel logs:
```bash
tail -f storage/logs/laravel.log
```

### Check recent data:
```bash
curl "http://yourdomain.com/api/water-quality/recent?device_id=WQM001&hours=1"
```

## Security Considerations

1. Use HTTPS in production
2. Keep your API key secure
3. Consider rate limiting for public endpoints
4. Monitor API usage and set up alerts

## Next Steps

1. Run the migration: `php artisan migrate`
2. Configure your domain and SSL certificate
3. Update Arduino code with your settings
4. Test the system with your sensors
5. Consider building a web dashboard to visualize the data

The system is now ready to receive and store water quality data from your Arduino device!