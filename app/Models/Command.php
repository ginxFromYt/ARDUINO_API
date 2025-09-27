<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\Factories\HasFactory;

class Command extends Model
{
    use HasFactory;

    protected $fillable = [
        'door_lock_id',
        'command',
        'executed',
    ];

    public function doorLock()
    {
        return $this->belongsTo(DoorLock::class);
    }
}
