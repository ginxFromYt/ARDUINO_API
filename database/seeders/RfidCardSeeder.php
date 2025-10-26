<?php

namespace Database\Seeders;

use Illuminate\Database\Seeder;
use App\Models\RfidCard;

class RfidCardSeeder extends Seeder
{
    /**
     * Run the database seeds.
     */
    public function run(): void
    {
        // Clean the table
        RfidCard::truncate();

        // Insert new RFID cards
        RfidCard::create([
            'user_id' => 1, // Admin user
            'uid' => '42:af:93:ab',
            'name' => 'Admin Card',
        ]);

        RfidCard::create([
            'user_id' => 2, // Regular user
            'uid' => '73:e0:d4:e4',
            'name' => 'Access Card 1',
        ]);

        RfidCard::create([
            'user_id' => 2, // Regular user
            'uid' => '13:51:a9:e2',
            'name' => 'Access Card 2',
        ]);
    }
}