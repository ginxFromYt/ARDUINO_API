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

Route::view('dashboard', 'dashboard')
    ->middleware(['auth', 'verified'])
    ->name('dashboard');


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

        return back()->with('status', 'Command sent successfully.');
    })->name('esp32.sendCommand');
});

require __DIR__.'/auth.php';
