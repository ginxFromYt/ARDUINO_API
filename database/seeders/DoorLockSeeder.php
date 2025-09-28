<?php

namespace Database\Seeders;

use Illuminate\Database\Seeder;
use App\Models\DoorLock;

class DoorLockSeeder extends Seeder
{
    /**
     * Run the database seeds.
     */
    public function run(): void
    {
        DoorLock::create([
            'user_id' => 2, // Adjust to match your test/admin user
            'status' => 'locked',
            'last_command' => null,
        ]);
    }
}
