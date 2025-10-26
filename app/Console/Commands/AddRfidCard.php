<?php

namespace App\Console\Commands;

use Illuminate\Console\Command;
use App\Models\User;
use App\Models\RfidCard;

class AddRfidCard extends Command
{
    /**
     * The name and signature of the console command.
     *
     * @var string
     */
    protected $signature = 'rfid:add {uid} {--name=} {--user=2}';

    /**
     * The console command description.
     *
     * @var string
     */
    protected $description = 'Add an RFID card to the database';

    /**
     * Execute the console command.
     */
    public function handle()
    {
        $uid = $this->argument('uid');
        $name = $this->option('name') ?? 'RFID Card';
        $userId = $this->option('user');

        // Check if user exists
        $user = User::find($userId);
        if (!$user) {
            $this->error("User with ID {$userId} not found!");
            return 1;
        }

        // Check if UID already exists
        $existingCard = RfidCard::where('uid', $uid)->first();
        if ($existingCard) {
            $this->error("RFID card with UID {$uid} already exists!");
            return 1;
        }

        // Create the RFID card
        $card = RfidCard::create([
            'user_id' => $userId,
            'uid' => $uid,
            'name' => $name,
        ]);

        $this->info("✅ RFID card added successfully!");
        $this->info("UID: {$card->uid}");
        $this->info("Name: {$card->name}");
        $this->info("User: {$user->name}");

        return 0;
    }
}
