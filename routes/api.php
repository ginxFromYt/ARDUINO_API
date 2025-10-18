<?php

use Illuminate\Support\Facades\Route;
use App\Http\Controllers\Api\AuthController;
use App\Http\Controllers\Api\DoorLockController;
use App\Http\Controllers\Api\WaterQualityController;

Route::post('/login', [AuthController::class, 'login']);

Route::middleware('auth:sanctum')->group(function () {
    Route::post('/door/command', [DoorLockController::class, 'sendCommand']);
    Route::get('/door/status', [DoorLockController::class, 'getStatus']);
});

// ESP32 endpoints (no auth)
Route::middleware('api.key')->group(function () {
    Route::post('/door/status', [DoorLockController::class, 'updateStatus']);
    Route::get('/door/command', [DoorLockController::class, 'getPendingCommand']);
    Route::post('/door/validate-rfid', [DoorLockController::class, 'validateRfid']);
    
    // Water Quality Monitoring endpoints
    Route::post('/water-quality/data', [WaterQualityController::class, 'store']);
    Route::get('/water-quality/status', [WaterQualityController::class, 'deviceStatus']);
});

// Public water quality endpoints (for monitoring/dashboard)
Route::prefix('water-quality')->group(function () {
    Route::get('/latest', [WaterQualityController::class, 'getLatest']);
    Route::get('/recent', [WaterQualityController::class, 'getRecent']);
});

