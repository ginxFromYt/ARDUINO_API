<?php

namespace Database\Seeders;

use Illuminate\Database\Console\Seeds\WithoutModelEvents;
use Illuminate\Database\Seeder;

class WaterQualityAdminSeeder extends Seeder
{
    /**
     * Run the database seeds.
     */
    public function run(): void
    {
        // Create or update the Water Quality Monitoring Admin user
        \App\Models\User::updateOrCreate(
            [
                'email' => 'wqmadmin@mail.com'
            ],
            [
                'name' => 'Water Quality Monitoring Admin',
                'password' => bcrypt('wqmadmin2025***'),
                'is_admin' => true,
                'email_verified_at' => now(),
            ]
        );

        $this->command->info('Water Quality Monitoring Admin user created/updated successfully!');
    }
}
