<?php

use Illuminate\Support\Facades\Route;
use App\Http\Controllers\Api\AuthController;
use App\Http\Controllers\Api\DoorLockController;

Route::post('/login', [AuthController::class, 'login']);

Route::middleware('auth:sanctum')->group(function () {
    Route::post('/door/command', [DoorLockController::class, 'sendCommand']);
    Route::get('/door/status', [DoorLockController::class, 'getStatus']);
});

// ESP32 endpoints (no auth)
Route::middleware('api.key')->group(function () {
    Route::post('/door/status', [DoorLockController::class, 'updateStatus']);
    Route::get('/door/command', [DoorLockController::class, 'getPendingCommand']);
});
