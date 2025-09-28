<?php

namespace Database\Seeders;

use App\Models\User;
// use Illuminate\Database\Console\Seeds\WithoutModelEvents;
use Illuminate\Database\Seeder;

class DatabaseSeeder extends Seeder
{
    /**
     * Seed the application's database.
     */
    public function run(): void
    {
        // User::factory(10)->create();

        User::factory()->create([
            'name' => 'Test User',
            'email' => 'esp32test@example.com',
            'password' => bcrypt('esp32password2025'),
        ]);

        User::factory()->create([
            'name' => 'Admin User',
            'email' => 'homeadmin@mail.com',
            'password' => bcrypt('Passworrd2025***'),
            'is_admin' => true,
        ]);

        $this->call(DoorLockSeeder::class);
    }
}
