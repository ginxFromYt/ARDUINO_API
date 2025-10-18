<?php

use App\Livewire\Settings\Appearance;
use App\Livewire\Settings\Password;
use App\Livewire\Settings\Profile;
use App\Livewire\Settings\TwoFactor;
use Illuminate\Support\Facades\Route;
use Laravel\Fortify\Features;

Route::get('/', function () {
    return view('welcome');
})->name('home');

// Smart dashboard redirect based on user role
Route::middleware(['auth', 'verified'])->get('dashboard', function () {
    $user = auth()->user();
    
    // Redirect Water Quality Admin to their dashboard
    if ($user->email === 'wqmadmin@mail.com') {
        return redirect()->route('water-quality.dashboard');
    }
    
    // Show door lock dashboard for door lock admin and others
    return view('dashboard');
})->name('dashboard');

// Water Quality Monitoring routes (protected by role middleware)
Route::middleware(['auth', 'verified', 'role:water-quality-admin'])->prefix('water-quality')->name('water-quality.')->group(function () {
    Route::get('/dashboard', [App\Http\Controllers\WaterQualityController::class, 'dashboard'])->name('dashboard');
    Route::get('/devices', [App\Http\Controllers\WaterQualityController::class, 'devices'])->name('devices');
    Route::get('/device/{deviceId}', [App\Http\Controllers\WaterQualityController::class, 'device'])->name('device');
    Route::get('/chart-data/{deviceId}', [App\Http\Controllers\WaterQualityController::class, 'chartData'])->name('chart-data');
    Route::get('/export/{deviceId}', [App\Http\Controllers\WaterQualityController::class, 'export'])->name('export');
});


Route::middleware(['auth'])->group(function () {
    Route::redirect('settings', 'settings/profile');

    Route::get('settings/profile', Profile::class)->name('settings.profile');
    Route::get('settings/password', Password::class)->name('settings.password');
    Route::get('settings/appearance', Appearance::class)->name('settings.appearance');

    Route::get('settings/two-factor', TwoFactor::class)
        ->middleware(
            when(
                Features::canManageTwoFactorAuthentication()
                    && Features::optionEnabled(Features::twoFactorAuthentication(), 'confirmPassword'),
                ['password.confirm'],
                [],
            ),
        )
        ->name('two-factor.show');

    // ESP32 command sender route
    Route::post('esp32/send-command', function (\Illuminate\Http\Request $request) {
        $request->validate([
            'command' => 'required|in:lock,unlock,status',
        ]);

        $doorLock = \App\Models\DoorLock::where('user_id', \Illuminate\Support\Facades\Auth::id())->firstOrFail();

        \App\Models\Command::create([
            'door_lock_id' => $doorLock->id,
            'command' => $request->command,
            'executed' => false,
        ]);

        $doorLock->last_command = $request->command;
        $doorLock->save();

        return back()->with('status', 'Command sent successfully.')->with('last_command', $request->command);
    })->name('esp32.sendCommand');
});

require __DIR__.'/auth.php';
